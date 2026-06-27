///////////////////////////////////////////////////////////////////////////////
/// @file error_types.h
/// @brief Custom error/exception classes
///
/// MAPS TO ORIGINAL: src/types/errors.ts
///
/// WHAT TO LEARN:
///   - C++ exceptions: https://learncpp.com/cpp-tutorial/exceptions/
///   - Custom exception classes
///   - std::runtime_error inheritance
///////////////////////////////////////////////////////////////////////////////
#pragma once
#include <optional>
#include <stdexcept>
#include <string>
#include "agent_types.h"

namespace codegraph
{
	struct AgentBusyDetails
	{
		std::string agentId;
		AgentStatus status;
		std::string reason;
		std::optional<int> queueLength;
		std::optional<int> maxQueue;
		std::optional<int> retryAfterMs;
		std::optional<std::string> taskId;
		std::optional<double> memoryUsageMB;
		std::optional<double> memoryLimitMB;
	};


	class AgentBusyError: public std::runtime_error
	{
	public:
		const AgentBusyDetails details;
		AgentBusyError(const AgentBusyDetails& details);
	};
}
