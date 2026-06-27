#include "error_types.h"

using namespace codegraph;

AgentBusyError::AgentBusyError(const AgentBusyDetails& details)
	: std::runtime_error("Agent" + details.agentId + "is busy (" + details.reason + ")"), 
	details(details)
{
	
}
