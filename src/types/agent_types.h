///////////////////////////////////////////////////////////////////////////////
/// @file agent_types.h
/// @brief Types for the multi-agent system
///
/// MAPS TO ORIGINAL: src/types/agent.ts
///
/// These define the "workers" in the system. Each agent has a type,
/// a status, and processes tasks from a queue.
///////////////////////////////////////////////////////////////////////////////
#pragma once

#include <string>
#include <optional>
#include <any>
#include <cstdint>
#include <future>
#include <memory>
#include <unordered_map>
#include <chrono>

namespace codegraph {
	using TimePoint = std::chrono::steady_clock::time_point;


// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AgentType — What kind of worker is this?
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
enum class AgentType {
    Coordinator,    // The boss — delegates tasks to other agents
    Parser,       // Reads code files and extracts entities
    Indexer,      // Saves parsed entities into the database
    Semantic,     // Generates embeddings for vector search
    Query,        // Handles search queries from AI clients
    Dev,          // Code analysis and refactoring tasks
    Dora,         // Research and documentation tasks
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AgentStatus — Current state of an agent
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
enum class AgentStatus {
    Idle,         // Waiting for work
  // Currently working on a task
    Busy,         // Queue is full, can't accept more
    Error,        // Something went wrong
    Shutdown,     // Agent is shutting down
};
struct AgentCapabilities {
    int max_Concurrency;
    double memoryLimit;
    std::optional<std::vector<int>> cpuAffinity;
    double priority;
};

template <typename T = std::any>
struct AgentMessage {
    std::string id;
    std::string from;
    std::string to;
    std::string type;
    T payload;
    uint64_t timestamp;
    std::optional<std::string> correlationId;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AgentTask — A unit of work for an agent (Command Pattern)
//
// The Conductor creates these and puts them in an agent's queue.
// The agent picks them up and processes them.
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct AgentTask {
    std::string id;
    std::string type;       // "parse_file", "index_entities", etc.
    int         priority = 0; // Higher = more urgent
    std::any payload;    // TODO: The actual task data (uncomment when needed)
    int64_t     createdAt = 0;
     // Which agent is handling this
    std::optional<TimePoint> startedAt;
    std::optional<TimePoint> completedAt;
    std::optional<std::string> error;
    std::optional<std::any> result;
};

struct Agent {
    std::string id;
    AgentType type;
    AgentStatus status;
    AgentCapabilities capabilities;
    // Lifecycle Methods
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    // Task processing
    virtual bool canHandle(const AgentTask& task) = 0;
    virtual std::future <std::any > process(AgentTask& task) = 0;
    // Communication
    virtual std::future<void> send(AgentMessage<>& message) = 0;
    virtual std::future<void> receive(AgentMessage<>& message) = 0;
    // Resource Management 
    virtual double getMemoryUsage() = 0;
    virtual double getCpuUsage() = 0;
    virtual std::vector<AgentTask> getTaskQueue() = 0;
    virtual ~Agent() = default;
};

struct AgentPool {
protected:
    std::unordered_map < std::string, std::unique_ptr<Agent>> agents;
public:
    virtual ~AgentPool() = default;
    virtual void registerAgent(std::unique_ptr<Agent> agent) = 0;
    virtual void unregisterAgent(std::string agentID) = 0;
    
    virtual Agent* getAgent(const std::string& id) = 0;
    virtual std::vector<Agent> getAgentByType(AgentType type) = 0;
    virtual Agent* getAvailableAgent(AgentType type) = 0;

    virtual std::future<void> broadcast(AgentMessage<> message) = 0;
    virtual std::future<Agent*> route(const AgentTask& task) = 0;
};

struct ResourceConstraints {
    double maxMemoryMB = 0;
    int maxCpuPercent = 0;
    int maxConcurrentAgents = 0;
    int maxTaskQueueSize = 0;
};

// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
// AgentMetrics — Performance statistics for an agent
// ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
struct AgentMetrics {
    std::string agentId;
    int         tasksProcessed = 0;
    int         tasksSucceeded = 0;
    int         tasksFailed    = 0;
    int         averageProcessingTime    = 0;
    double      currentMemoryMB = 0.0;
    double      currentCpuPercent  = 0.0;
    TimePoint   lastActivity;
};

} // namespace codegraph
