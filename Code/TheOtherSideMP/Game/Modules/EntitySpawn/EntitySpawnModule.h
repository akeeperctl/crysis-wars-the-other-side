// ReSharper disable CppClangTidyCppcoreguidelinesSpecialMemberFunctions
#pragma once

#include "../GenericModule.h"

/**
 * \brief Модуль создания сущностей, используемых в моде The Other Side
 * \note Также модуль предназначен для воскрешения сущностей, удаленных во время работы консольной команды sv_restart.
 */
class CTOSEntitySpawnModule final :
	public CTOSGenericModule  
{
public:
	CTOSEntitySpawnModule();
	~CTOSEntitySpawnModule() override;

	//ITOSGameModule
	void OnExtraGameplayEvent(IEntity* pEntity, const STOSGameEvent& event) override;
	const char* GetName() override {return "CTOSEntitySpawnModule";};
	void Init() override;
	void Update(float frametime) override;
	void Serialize(TSerialize ser) override;
	//~ITOSGameModule

	/**
	 * \brief Создаёт сущность по определенным параметрам
	 * \param params - параметры создания сущности
	 * \return Если успешно, то указатель IEntity* на созданную сущность, иначе nullptr
	 */
	IEntity* SpawnEntity(SEntitySpawnParams& params) const;

private:

};