#include "ImGuiLayer.hpp"

#include "imgui/imgui_impl_opengl3.h"

//Temporary
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Flow3D/Core.hpp"
#include "Flow3D/Application.hpp"
#include "Flow3D/Log.hpp"

namespace Flow {

	// Helper to display a little (?) mark which shows a tooltip when hovered.
	static void ShowHelpMarker(const char* desc)
	{
		ImGui::TextDisabled("(?)");
		if (ImGui::IsItemHovered())
		{
			ImGui::BeginTooltip();
			ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
			ImGui::TextUnformatted(desc);
			ImGui::PopTextWrapPos();
			ImGui::EndTooltip();
		}
	}

	ImGuiLayer::ImGuiLayer() {}

	void ImGuiLayer::OnAttach()
	{
		ImGui::CreateContext();
		ImGui::StyleColorsDark();

		ImGuiIO& io = ImGui::GetIO();
		io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
		io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;

		//Temporary:
		// Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
		io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
		io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
		io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
		io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
		io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
		io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
		io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
		io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
		io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
		io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
		io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
		io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
		io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
		io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
		io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
		io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
		io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
		io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
		io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
		io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
		io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

		ImGui_ImplOpenGL3_Init("#version 410");

		m_Logger = std::make_unique<Flow3DLog>();
		m_Inspector = std::make_unique<Flow3DInspector>();
	}

	void ImGuiLayer::OnDetach()
	{
	}

	void ImGuiLayer::OnUpdate(double deltaTime)
	{
		ImGuiIO& io = ImGui::GetIO();
		Application& app = Application::Get();
		int width = app.GetWindow().GetWidth();
		int height = app.GetWindow().GetHeight();
		io.DisplaySize = ImVec2((float)width, (float)height);

		float time = (float)glfwGetTime();
		io.DeltaTime = m_Time > 0.0f ? (time - m_Time) : (1.0f / 60.0f);
		m_Time = time;

		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();
		
		static bool show = true;
		
		ImGui::ShowDemoWindow(&show);

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("Flow3D"))
			{
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Settings"))
			{
				Skybox& skybox = app.GetCurrentScene().GetSkybox();
				if (ImGui::MenuItem("Skybox", NULL, skybox.IsShown()))
					skybox.ToggleShow();

				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		ImGui::SetNextWindowContentSize(ImVec2(240.0f, 960.0f));
		if (ImGui::Begin("Hierarchy", &show, ImGuiWindowFlags_NoCollapse))
		{
			static char nameBuffer[32] = "GameObject";
			if (ImGui::InputText("##hidelabel", nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_EnterReturnsTrue))
			{
				FLOW_CORE_INFO("name is: {0}", nameBuffer);
			}
			ImGui::SameLine(0, 20.0f);
			if (ImGui::Button("ADD GO"))
			{
				FLOW_CORE_INFO("new go name would be {0}", nameBuffer);
			}

			ImGui::Separator();
			ImGui::Columns(1);

			// Iterate dummy objects with dummy members (all the same data)
			std::vector<GameObject*> allGameObjects = Application::Get().GetAllGameObjects();
			for (int i = 0; i < allGameObjects.size(); i++)
				ShowGameObject(allGameObjects[i]->GetName().c_str(), i, *allGameObjects[i]);

			ImGui::Separator();

			ImGui::End();
		}
			
		ImGui::SetNextWindowContentSize(ImVec2(1280.0f, 720.0f));
		// create our ImGui window
		if (ImGui::Begin("Viewport", &show, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
		{
			//get the mouse position
			ImVec2 pos = ImGui::GetCursorScreenPos();

			//pass the texture of the FBO
			//app.GetRenderTexture() is the texture of the FBO
			//the next parameter is the upper left corner for the uvs to be applied at
			//the third parameter is the lower right corner
			//the last two parameters are the UVs
			//they have to be flipped (normally they would be (0,0);(1,1) 
			ImGui::GetWindowDrawList()->AddImage(
				(void *)app.GetRenderTexture(),
				ImVec2(ImGui::GetCursorScreenPos()),
				ImVec2(ImGui::GetCursorScreenPos().x + 1280,
					ImGui::GetCursorScreenPos().y + 720), ImVec2(0, 1), ImVec2(1, 0));

			//we are done working with this window
			ImGui::End();
		}	

		m_Inspector->Draw();

		m_Logger->Draw("Flow3D Log");

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	void ImGuiLayer::ShowGameObject(const char* prefix, int uid, GameObject& child)
	{
		ImGui::PushID(uid);                      // Use object uid as identifier. Most commonly you could also use the object pointer as a base ID.
		ImGui::AlignTextToFramePadding();  // Text and Tree nodes are less high than regular widgets, here we add vertical spacing to make the tree lines equal high.
		const std::vector<GameObject*>& children = child.GetChildren();
		if (children.size() > 0)
		{
			bool node_open = ImGui::TreeNodeEx(prefix, ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow);
			if (ImGui::IsItemClicked())
				m_Inspector->SetGameObject(&child);			

			ImGui::NextColumn();
			if (node_open)
			{
				for (int i = 0; i < children.size(); i++)
					ShowGameObject(children[i]->GetName().c_str(), i, *children[i]);
				ImGui::TreePop();
			}
		}
		else
		{
			ImGui::TreeNodeEx(prefix, ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Bullet);
			if (ImGui::IsItemClicked())
				m_Inspector->SetGameObject(&child);
				
			ImGui::NextColumn();
		}

		ImGui::PopID();
	}

	void ImGuiLayer::OnEvent(Event& event)
	{
		// only dispatch events to imgui when it is active; at the moment this means that the demo window is shown
		if (m_ReceiveEvents)
		{
			EventDispatcher dispatcher(event);
			dispatcher.Dispatch<MouseButtonPressedEvent>(FLOW_BIND_EVENT_FUNCTION(ImGuiLayer::OnMouseButtonPressedEvent));
			dispatcher.Dispatch<MouseButtonReleasedEvent>(FLOW_BIND_EVENT_FUNCTION(ImGuiLayer::OnMouseButtonReleasedEvent));
			dispatcher.Dispatch<MouseMovedEvent>(FLOW_BIND_EVENT_FUNCTION(ImGuiLayer::OnMouseMovedEvent));
			dispatcher.Dispatch<MouseScrolledEvent>(FLOW_BIND_EVENT_FUNCTION(ImGuiLayer::OnMouseScrolledEvent));
			dispatcher.Dispatch<KeyPressedEvent>(FLOW_BIND_EVENT_FUNCTION(ImGuiLayer::OnKeyPressedEvent));
			dispatcher.Dispatch<KeyReleasedEvent>(FLOW_BIND_EVENT_FUNCTION(ImGuiLayer::OnKeyReleasedEvent));
			dispatcher.Dispatch<KeyTypedEvent>(FLOW_BIND_EVENT_FUNCTION(ImGuiLayer::OnKeyTypedEvent));
			dispatcher.Dispatch<WindowResizeEvent>(FLOW_BIND_EVENT_FUNCTION(ImGuiLayer::OnWindowResizeEvent));		}
		
	}

	bool ImGuiLayer::OnMouseButtonPressedEvent(MouseButtonPressedEvent & event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[event.GetMouseButton()] = true;

		return m_ReceiveEvents;
	}

	bool ImGuiLayer::OnMouseButtonReleasedEvent(MouseButtonReleasedEvent & event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseDown[event.GetMouseButton()] = false;

		return m_ReceiveEvents;
	}

	bool ImGuiLayer::OnMouseMovedEvent(MouseMovedEvent & event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MousePos = ImVec2(event.GetX(), event.GetY());

		return m_ReceiveEvents;
	}

	bool ImGuiLayer::OnMouseScrolledEvent(MouseScrolledEvent & event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.MouseWheel += event.GetYOffset();
		io.MouseWheelH += event.GetXOffset();

		return m_ReceiveEvents;
	}

	bool ImGuiLayer::OnKeyPressedEvent(KeyPressedEvent & event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[event.GetKeyCode()] = true;

		io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
		io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
		io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
		return m_ReceiveEvents;
	}

	bool ImGuiLayer::OnKeyReleasedEvent(KeyReleasedEvent & event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.KeysDown[event.GetKeyCode()] = false;

		return m_ReceiveEvents;
	}

	bool ImGuiLayer::OnKeyTypedEvent(KeyTypedEvent & event)
	{
		ImGuiIO& io = ImGui::GetIO();
		int keycode = event.GetKeyCode();
		if (keycode > 0 && keycode < 0x10000)
			io.AddInputCharacter((unsigned short)keycode);

		return m_ReceiveEvents;
	}

	bool ImGuiLayer::OnWindowResizeEvent(WindowResizeEvent & event)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)event.GetWidth(), (float)event.GetHeight());
		io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);
		glViewport(0, 0, event.GetWidth(), event.GetHeight());

		return m_ReceiveEvents;
	}
}