#pragma once

#include "Alien.h"

class CTOSAlien: public CAlien  // NOLINT(cppcoreguidelines-special-member-functions)
{
public:
	friend class CTOSMasterClient;

	CTOSAlien();
	~CTOSAlien() ;
	
	//CTOSActor
	void ProcessEvent(SEntityEvent& event) ;
	void PrePhysicsUpdate();
	void     SetHealth(int health) ;
	Matrix33 GetViewMtx() ;
	Matrix33 GetBaseMtx() ;
	Matrix33 GetEyeMtx() ;
	void	 Kill() ;
	void	 Revive(bool fromInit) ;
	bool	 ApplyActions(int actions);
	void SetParams(SmartScriptTable& rTable, bool resetFirst);
	IActorMovementController* CreateMovementController();
	void PostInit(IGameObject* pGameObject);
	void PostPhysicalize();
	void Update(SEntityUpdateContext& ctx, int updateSlot) ;
	bool NetSerialize(TSerialize ser, EEntityAspects aspect, uint8 profile, int flags) ;
	//~CTOSActor

	//ITOSMasterControllable
	void ApplyMasterMovement(const Vec3& delta) ;
	//~ITOSMasterControllable

protected:

	// Used to serialize special movement requests (e.g. SmartObject actortarget usage and such)
	struct SSpecialMovementRequestParams
	{
		SSpecialMovementRequestParams() {};
		SSpecialMovementRequestParams(uint32 reqFlags, const SActorTargetParams& actorTarget, const string& animation) 
			: flags(reqFlags)
			, targetParams(actorTarget)
		{
			// HACK: The structure copying is retarded and this needs to be done...
			//targetAnimation = animation;
		};

		uint32 flags;
		SActorTargetParams targetParams;
		//string targetAnimation; // why isnt it serializing?

		void SerializeWith(TSerialize ser)
		{

			ser.Value("flags", flags);

			//if ((flags & CMovementRequest::eMRF_ActorTarget))
			{

				ser.Value("location", targetParams.location);
				ser.Value("direction", targetParams.direction);
				ser.Value("vehicleName", targetParams.vehicleName);
				ser.Value("vehicleSeat", targetParams.vehicleSeat);
				ser.Value("speed", targetParams.speed);
				ser.Value("directionRadius", targetParams.directionRadius);
				ser.Value("locationRadius", targetParams.locationRadius);
				ser.Value("startRadius", targetParams.startRadius);
				ser.Value("signalAnimation", targetParams.signalAnimation);
				ser.Value("projectEnd", targetParams.projectEnd);
				ser.Value("navSO", targetParams.navSO);
				ser.Value("animation", targetParams.animation);
				ser.Value("stance", (int&)targetParams.stance);
				ser.Value("triggerUser", (int&)targetParams.triggerUser);
			}
			
		}
	};
	DECLARE_CLIENT_RMI_NOATTACH(ClSpecialMovementRequest, SSpecialMovementRequestParams, eNRT_ReliableOrdered);
	void SendSpecialMovementRequest(uint32 reqFlags, const SActorTargetParams& targetParams);

private:
};