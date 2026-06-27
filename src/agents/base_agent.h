#pragma once

#include<string>
#include<vector>
#include<future>
#include "types/agent_types.h"
#include "types/error_types.h"
#include <functional>
#include<any>
#include<iostream>


namespace codegraph {
	struct TaskCompletedEvent {
		std::string agentId;
		AgentTask   task;
	};
	struct TaskFailedEvent {
		std::string    agentId;
		AgentTask      task;
		std::exception_ptr error;  // C++ way to carry a caught exception
	};
	
	class EventEmitter {
	public:
		void on(const std::string& event, std::function<void(std::any)> callback) {
			std::lock_guard<std::mutex> lock(listenerMutex_);
			listeners_[event].push_back(callback);
		}
		void emit(const std::string& event, std::any data = {}) {
			std::lock_guard<std::mutex>lock(listenerMutex_);
			auto it = listeners_.find(event);
			if (it != listeners_.end()) {
				for (auto& cb : it->second) {
					cb(data);
				}
			}
		}
		virtual ~EventEmitter() = default;
		
	private:
		std::unordered_map<std::string, std::vector<std::function<void(std::any)>>> listeners_;
		std::mutex listenerMutex_;
	}; 
	class BaseAgent : public EventEmitter , public Agent {
	public:
		const std::string id;
		const AgentType type;
		AgentStatus status;
		const AgentCapabilities capabilities;

		BaseAgent(AgentType type, AgentCapabilities capabilities);

		virtual ~BaseAgent();
		void initialize() override;
		void shutdown() override;
		bool canHandle(const AgentTask& task) override;
		std::future <std::any > process(AgentTask& task) override;
		// Communication
		std::future<void> send(AgentMessage<>& message) override;
		std::future<void> receive(AgentMessage<>& message) override;
		// Resource Management 
		double getMemoryUsage() override;
		double getCpuUsage() override;
		std::vector<AgentTask> getTaskQueue() override;
		AgentMetrics getMetrics() const;

	protected:
		std::vector<AgentTask> taskQueue;
		std::optional<AgentTask> currentTask;
		AgentMetrics metrics;
		double memoryUsage = 0;
		double cpuUsage = 0;
		virtual std::future<void> onInitialize() = 0;
		virtual std::future<void> onShutdown() = 0;
		virtual bool canProcessTask(const AgentTask& task) = 0;
		virtual std::future<std::any> processTask(AgentTask& task) = 0;
		virtual std::future<void> handleMessage(AgentMessage<>& message) = 0;
		
	private:
		std::optional<AgentBusyDetails> lastRejection;
		std::thread monitorThread;
		std::atomic<bool>monitorRunning{ false };

		void startResourceMonitoring();
		void updateResourceUsage();
		void updateAverageProcessingTime(double duration);
	};
}




