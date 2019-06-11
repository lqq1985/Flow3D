#pragma once

#include "Component.hpp"
#include "Flow3D/Input.hpp"
#include "Flow3D/Application.hpp"

namespace Flow {

	// Constructor: GameObjectToggler(GameObject& gameObject, std::string gameObjectName, bool enabled = true)
	// gives an object the functionality and data to toggle the active state of a given GameObject
	class GameObjectToggler : public Component
	{
		CLASS_DECLARATION(GameObjectToggler)

	public:
		GameObjectToggler(GameObject& gameObject, std::string gameObjectName, bool enabled = true)
			: Component(gameObject, enabled, "GameObjectToggler"), m_Input(Input::Get()), m_GameObjectName(gameObjectName)
		{
			m_GameObjectToToggle = Application::Get().GetCurrentScene().FindGameObject(gameObjectName);
		}

		virtual void OnEvent(Event& e) override
		{
			EventDispatcher dispatcher(e);
			dispatcher.Dispatch<KeyPressedEvent>(FLOW_BIND_EVENT_FUNCTION(GameObjectToggler::OnKeyPressed));
		}

		void SetGameObjectName(std::string newName)
		{
			m_GameObjectName = newName;
			m_GameObjectToToggle = Application::Get().GetCurrentScene().FindGameObject(m_GameObjectName);
		}
		const std::string GetGameObjectName() const { return m_GameObjectName; }

	private:
		Input& m_Input;
		std::string m_GameObjectName;
		GameObject* m_GameObjectToToggle;

		bool OnKeyPressed(KeyPressedEvent& e)
		{
			if (e.GetKeyCode() == (int)Keycode::G)
			{
				if (m_GameObjectToToggle != nullptr)
					m_GameObjectToToggle->SetActive(!m_GameObjectToToggle->GetIsActive());
			}				

			return false; // should not block other events right now because it is only for testing
		}
	};
}
