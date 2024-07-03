#pragma once
#include <string>
#include <vector>

class SceneObject;
struct Packet;

namespace NetworkEvent
{
	void TriggerNetworkEvent(std::string Name, std::vector<std::string> Arguments, SceneObject* Target, uint64_t TargetClient);

	void HandleNetworkEvent(Packet* Data);
	void HandleEventAccept(Packet* Data);

	void Update();

	void ClearEventsFor(uint64_t PlayerID);
}