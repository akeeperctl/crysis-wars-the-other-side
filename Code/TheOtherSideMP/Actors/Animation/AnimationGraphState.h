#pragma once
#pragma once

#include <IAnimationGraph.h>

class CTOSActor;

class CAnimationGraphState final : public IAnimationGraphState
{
	friend class CTOSActor;

	CAnimationGraphState(CTOSActor* pActor, IAnimationGraphState* pState)
		:
		m_pOwner(pActor),
		m_pAnimationGraphState(pState) { }

	~CAnimationGraphState() { }

public:
	// IAnimationGraphState

	// recurse setting. query mechanism needs to be wrapped by wrapper.
	// Associated QueryID will be given to QueryComplete when ALL layers supporting the input have reached their matching states.
	// wrapper generates it's own query IDs which are associated to a bunch of sub IDs with rules for how to handle the sub IDs into wrapped IDs.
	bool SetInput(InputID id, float value, TAnimationGraphQueryID* pQueryID = nullptr) override;
	bool SetInput(InputID id, int value, TAnimationGraphQueryID* pQueryID = nullptr) override;
	bool SetInput(InputID id, const char* value, TAnimationGraphQueryID* pQueryID = nullptr) override;
	// SetInputOptional is same as SetInput except that it will not set the default input value in case a non-existing value is passed
	bool SetInputOptional(const InputID id, const char* value, TAnimationGraphQueryID* pQueryID = nullptr) override
	{
		return m_pAnimationGraphState->SetInputOptional(id, value, pQueryID);
	};

	void ClearInput(const InputID id) override
	{
		m_pAnimationGraphState->ClearInput(id);
	};

	void LockInput(const InputID id, const bool locked) override
	{
		m_pAnimationGraphState->LockInput(id, locked);
	};

	// assert all equal, use any (except if signalled, then return the one not equal to default, or return default of all default)
	void GetInput(const InputID id, char* name) const override
	{
		return m_pAnimationGraphState->GetInput(id, name);
	};

	// AND all layers
	bool IsDefaultInputValue(const InputID id) const override
	{
		return m_pAnimationGraphState->IsDefaultInputValue(id);
	};

	// returns NULL if InputID is out of range
	const char* GetInputName(const InputID id) const override
	{
		return m_pAnimationGraphState->GetInputName(id);
	};

	// When QueryID of SetInput (reached queried state) is emitted this function is called by the outside, by convention(verify!).
	// Remember which layers supported the SetInput query and emit QueryLeaveState QueryComplete when all those layers have left those states.
	void QueryLeaveState(TAnimationGraphQueryID* pQueryID) override
	{
		m_pAnimationGraphState->QueryLeaveState(pQueryID);
	};

	// assert all equal, forward to all layers, complete when all have changed once (trivial, since all change at once via SetInput).
	// (except for signalled, forward only to layers which currently are not default, complete when all those have changed).
	void QueryChangeInput(const InputID id, TAnimationGraphQueryID* query) override
	{
		m_pAnimationGraphState->QueryChangeInput(id, query);
	};

	// Just register and non-selectivly call QueryComplete on all listeners (regardless of what ID's they are actually interested in).
	void AddListener(const char* name, IAnimationGraphStateListener* pListener) override
	{
		m_pAnimationGraphState->AddListener(name, pListener);
	};

	void RemoveListener(IAnimationGraphStateListener* pListener) override
	{
		m_pAnimationGraphState->RemoveListener(pListener);
	};

	// Not used
	bool DoesInputMatchState(const InputID id) override
	{
		return m_pAnimationGraphState->DoesInputMatchState(id);
	};

	// TODO: This should be turned into registered callbacks or something instead (look at AnimationGraphStateListener).
	// Use to access the SelectLocomotionState() callback in CAnimatedCharacter.
	// Only set for fullbody, null for upperbody.
	void SetAnimatedCharacter(class CAnimatedCharacter* animatedCharacter, const int layerIndex, IAnimationGraphState* parentLayerState) override
	{
		m_pAnimationGraphState->SetAnimatedCharacter(animatedCharacter, layerIndex, parentLayerState);
	}

	// simply recurse
	bool Update() override
	{
		return m_pAnimationGraphState->Update();
	};

	void Release() override
	{
		m_pAnimationGraphState->Release();
		delete this;
	};

	void ForceTeleportToQueriedState() override
	{
		m_pAnimationGraphState->ForceTeleportToQueriedState();
	};

	// simply recurse (will be ignored by each layer individually if state not found)
	void PushForcedState(const char* state, TAnimationGraphQueryID* pQueryID = nullptr) override
	{
		m_pAnimationGraphState->PushForcedState(state, pQueryID);
	};

	// simply recurse
	void ClearForcedStates() override
	{
		m_pAnimationGraphState->ClearForcedStates();
	};

	// simply recurse
	void SetBasicStateData(const SAnimationStateData& data) override
	{
		m_pAnimationGraphState->SetBasicStateData(data);
	};

	// same as GetInput above
	float GetInputAsFloat(const InputID inputId) override
	{
		return m_pAnimationGraphState->GetInputAsFloat(inputId);
	};

	// wrapper generates it's own input IDs for the union of all inputs in all layers, and for each input it maps to the layer specific IDs.
	InputID GetInputId(const char* input) override
	{
		return m_pAnimationGraphState->GetInputId(input);
	};

	// simply recurse (preserve order), and don't forget to serialize the wrapper stuff, ID's or whatever.
	void Serialize(const TSerialize ser) override
	{
		m_pAnimationGraphState->Serialize(ser);
	};

	// simply recurse
	void SetAnimationActivation(const bool activated) override
	{
		m_pAnimationGraphState->SetAnimationActivation(activated);
	};

	bool GetAnimationActivation() override
	{
		return m_pAnimationGraphState->GetAnimationActivation();
	};

	// Concatenate all layers state names with '+'. Use only fullbody layer state name if upperbody layer is not allowed/mixed.
	const char* GetCurrentStateName() override
	{
		return m_pAnimationGraphState->GetCurrentStateName();
	};

	// don't expose (should only be called on specific layer state directly, by AGAnimation)
	//virtual void ForceLeaveCurrentState() = 0;
	//virtual void InvalidateQueriedState() = 0;

	// simply recurse
	void Pause(const bool pause, const EAnimationGraphPauser pauser) override
	{
		m_pAnimationGraphState->Pause(pause, pauser);
	};

	// is the same for all layers (equal assertion should not even be needed)
	bool IsInDebugBreak() override
	{
		return m_pAnimationGraphState->IsInDebugBreak();
	};

	// find highest layer that has output id, or null (this allows upperbody to override fullbody).
	// Use this logic when calling SetOutput on listeners.
	const char* QueryOutput(const char* name) override
	{
		return m_pAnimationGraphState->QueryOutput(name);
	};

	// Exact positioning: Forward to fullbody layer only (hardcoded)
	IAnimationSpacialTrigger* SetTrigger(const SAnimationTargetRequest& req, const EAnimationGraphTriggerUser user, TAnimationGraphQueryID* pQueryStart, TAnimationGraphQueryID* pQueryEnd) override
	{
		return m_pAnimationGraphState->SetTrigger(req, user, pQueryStart, pQueryEnd);
	};

	void ClearTrigger(const EAnimationGraphTriggerUser user) override
	{
		m_pAnimationGraphState->ClearTrigger(user);
	};

	const SAnimationTarget* GetAnimationTarget() override
	{
		return m_pAnimationGraphState->GetAnimationTarget();
	};

	void SetTargetPointVerifier(IAnimationGraphTargetPointVerifier* ptr) override
	{
		m_pAnimationGraphState->SetTargetPointVerifier(ptr);
	};

	bool IsUpdateReallyNecessary() override
	{
		return m_pAnimationGraphState->IsUpdateReallyNecessary();
	};

	// (only used by vehicle code) (to support simultaneous layer query, IAnimationGraphExistanceQuery must implement it).
	// Forward to fullbody layer only (hardcoded)
	IAnimationGraphExistanceQuery* CreateExistanceQuery() override
	{
		return m_pAnimationGraphState->CreateExistanceQuery();
	};

	// simply recurse
	void Reset() override
	{
		m_pAnimationGraphState->Reset();
	};

	// we've been idle for a while, try to catch up and disrespect blending laws
	// simply recurse
	void SetCatchupFlag() override
	{
		m_pAnimationGraphState->SetCatchupFlag();
	};

	// (hardcoded forward to fullbody layer only) (used for exact positioning trigger and PMC::UpdateMovementState()).
	Vec2 GetQueriedStateMinMaxSpeed() override
	{
		return m_pAnimationGraphState->GetQueriedStateMinMaxSpeed();
	};

	// simply recurse (hurry all layers, let them hurry independently where they can)
	void Hurry() override
	{
		m_pAnimationGraphState->Hurry();
	};

	// simply recurse (first person skippable states are skipped independently by each layer)
	void SetFirstPersonMode(const bool on) override
	{
		m_pAnimationGraphState->SetFirstPersonMode(on);
	};

	// simply recurse (will add all layer's containers to the sizer)
	void GetMemoryStatistics(ICrySizer* s) override
	{
		m_pAnimationGraphState->GetMemoryStatistics(s);
	};

	// the wrapper simply returns false
	bool IsMixingAllowedForCurrentState() const override
	{
		return m_pAnimationGraphState->IsMixingAllowedForCurrentState();
	};

	// used by CAnimationGraphStates
	bool IsSignalledInput(const InputID intputId) const override
	{
		return m_pAnimationGraphState->IsSignalledInput(intputId);
	};

	// ~IAnimationGraphState

private:
	// Pointer to the owner actor.
	CTOSActor* m_pOwner;
	// Pointer to the native IAnimationGraphState class implementation.
	IAnimationGraphState* m_pAnimationGraphState;
};
