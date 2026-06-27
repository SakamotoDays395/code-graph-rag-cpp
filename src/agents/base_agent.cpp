#include "base_agent.h"
#include<iostream>
#include<algorithm>
#include<random>
#include<sstream>

#include "types/error_types.h"
std::string agentTypeToString(codegraph::AgentType type); //implement in last 
 

using namespace  codegraph;

static std::string generateShortId()
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<uint32_t> dist(0, 0xFFFFFFFF);
	std::stringstream ss;
	ss << std::hex << dist(gen);
	return ss.str().substr(0, 8);
}

BaseAgent::BaseAgent(AgentType type, AgentCapabilities capabilities)
	: id(agentTypeToString(type) + "-" + generateShortId()),
		type(type), status(AgentStatus::Idle), capabilities(capabilities) 
{
	metrics.agentId = id;
	metrics.tasksProcessed = 0;
	metrics.tasksSucceeded = 0;
	metrics.tasksFailed = 0;
	metrics.averageProcessingTime = 0;
	metrics.currentMemoryMB = 0;
	metrics.currentCpuPercent = 0;
	metrics.lastActivity = std::chrono::steady_clock::now();
	startResourceMonitoring();
}
BaseAgent::~BaseAgent()
{
	monitorRunning = false;
	if (monitorThread.joinable())
	{
		monitorThread.join();
	}
}

void BaseAgent::initialize()
{
	std::cout << "[" << id << "] Initializing Agent...." << std::endl;
	status = AgentStatus::Idle;
	onInitialize();
	emit("initialized", id);
}

void BaseAgent::shutdown()
{
	std::cout << "[" << id << "] Shutting down agent..." << std::endl;
	status = AgentStatus::Shutdown;
	onShutdown();
	monitorRunning = false;
	if (monitorThread.joinable())
	{
		monitorThread.join();
	}
	emit("shutdown", id);
}

bool BaseAgent::canHandle(const AgentTask& task)
{
	lastRejection = std::nullopt;

	if (status != AgentStatus::Idle)
	{
		lastRejection = AgentBusyDetails{ id, status, "not_idle", static_cast<int>(taskQueue.size()), capabilities.max_Concurrency, 200 };
		return false;
	}

	if (static_cast<int>(taskQueue.size()) >= capabilities.max_Concurrency)
	{
		lastRejection = AgentBusyDetails{ id, status, "queue_full", static_cast<int>(taskQueue.size()), capabilities.max_Concurrency, 250 };
		return false;
	}
	if (memoryUsage> capabilities.memoryLimit * 0.9) {
		lastRejection = AgentBusyDetails{
			id, status, "memory_limit",
			static_cast<int>(taskQueue.size()),
			capabilities.max_Concurrency,
			500
		};
		return false;
	}
	bool canProcess = canProcessTask(task);
	
	if (!canProcess)
	{
		lastRejection= AgentBusyDetails{
			id, status, "unsupported_task",
			static_cast<int>(taskQueue.size()),
			capabilities.max_Concurrency
		};
	}
	return canProcess;
}

std::future<std::any> BaseAgent::process(AgentTask& task)
{
	return std::async(std::launch::async, [this, &task]() -> std::any {
		if (!canHandle(task)) {
			AgentBusyDetails details{
				id, status,
				lastRejection ? lastRejection->reason : "unknown",
				lastRejection ? lastRejection->queueLength : 0,
				lastRejection ? lastRejection->maxQueue : 0,
				lastRejection ? lastRejection->retryAfterMs : 300
			};
			throw AgentBusyError(details);
		}
		// lines 168-171: Enqueue and mark busy
		taskQueue.push_back(task);
		status = AgentStatus::Busy;
		currentTask = task;
		task.startedAt = std::chrono::steady_clock::now();

		struct Cleanup {
			BaseAgent& self;
			AgentTask& task;
			~Cleanup() {
				// C++20: clean one-liner replaces the ugly erase-remove
				std::erase_if(self.taskQueue,
					[&](const AgentTask& t) { return t.id == task.id; });
				self.currentTask = std::nullopt;
				if (self.taskQueue.empty()) {
					self.status = AgentStatus::Idle;
				}
				self.metrics.lastActivity = std::chrono::steady_clock::now();
				self.lastRejection = std::nullopt;
			}
		} cleanup{ *this, task };

		try {
			// line 174: Delegate to child's processTask()
			std::any result = processTask(task).get();
			// lines 175-180: Record success
			task.completedAt = std::chrono::steady_clock::now();
			task.result = result;
			metrics.tasksProcessed++;
			metrics.tasksSucceeded++;
			// Calculate duration in milliseconds
			auto durationMs = std::chrono::duration_cast<std::chrono::milliseconds>(*task.completedAt - *task.startedAt).count();
			updateAverageProcessingTime(static_cast<double>(durationMs));
			emit("task:completed", TaskCompletedEvent{id, task});
			return result;
		}
		catch (...) {
			// lines 184-192: Record failure
			task.completedAt = std::chrono::steady_clock::now();
			metrics.tasksProcessed++;
			metrics.tasksFailed++;
			emit("task:failed", TaskFailedEvent{id,task,std::current_exception()});
			throw;
		} 
	});
}





