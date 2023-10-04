#include "components_manager.h"

#include "runtime/editor/dialog/dialog_base.h"

#include "foundation/input/mouse.h"
#include "foundation/input/keyboard.h"
#include "foundation/graphics/viewport.h"
#include "foundation/graphics/textfield.h"

#include "runtime/editor/components/component_base.h"
#include "runtime/editor/visualizer_components/visualizer_component_base.h"
#include "runtime/emulation/cpumemory.h"

#include <vector>
#include "foundation/base/assert.h"
#include <algorithm>

using namespace Foundation;

namespace Editor
{
	ComponentsManager::ComponentsManager(Foundation::Viewport* inViewport, CursorControl* inCursorControl)
		: m_Viewport(inViewport)
		, m_CursorControl(inCursorControl)
		, m_EnabledInputGroups(0)
		, m_EnabledTabGroup(0)
		, m_FocusComponent(nullptr)
		, m_Suspended(false)
	{
		memset(m_TabGroupFocusComponent, 0, sizeof(m_TabGroupFocusComponent));
	}

	ComponentsManager::~ComponentsManager()
	{

	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::Clear()
	{
		SetComponentInFocus(nullptr);
		m_EnabledTabGroup = 0;
		memset(m_TabGroupFocusComponent, 0, sizeof(m_TabGroupFocusComponent));

		m_Components.clear();
		m_VisualizerComponents.clear();
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::Suspend()
	{
		if (!m_Suspended)
		{
			if (m_FocusComponent != nullptr)
				m_FocusComponent->ClearHasControl(*m_CursorControl);

			m_Suspended = true;
		}
	}


	void ComponentsManager::Resume()
	{
		if (m_Suspended)
		{
			if (m_FocusComponent != nullptr)
				m_FocusComponent->SetHasControl(ComponentBase::GetControlType::WasSuspended, *m_CursorControl);

			if (m_FocusComponent != nullptr)
			{
				m_CursorControl->SetTargetTextField(m_FocusComponent != nullptr ? m_FocusComponent->GetTextField() : nullptr);
				m_CursorControl->SetDimensions(CursorControl::Extent({ 1, 1 }));
			}

			m_Suspended = false;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::StartDialog(std::shared_ptr<DialogBase> inDialog)
	{
		if (!m_Suspended)
		{
			FOUNDATION_ASSERT(m_ActiveDialog == nullptr);
			Suspend();

			m_ActiveDialog = inDialog;
			m_ActiveDialog->Activate(m_Viewport, m_CursorControl);
		}
		else
		{
			if (m_ActiveDialog != nullptr)
			{
				FOUNDATION_ASSERT(m_ActiveDialog->IsDone());

				m_ActiveDialog->Deactivate(m_Viewport);
				m_ActiveDialog = inDialog;
				m_ActiveDialog->Activate(m_Viewport, m_CursorControl);
			}
		}
	}


	bool ComponentsManager::IsDisplayingDialog() const
	{
		return m_Suspended && m_ActiveDialog != nullptr;
	}


	void ComponentsManager::CancelDialog()
	{
		FOUNDATION_ASSERT(m_ActiveDialog != nullptr);
		FOUNDATION_ASSERT(m_Suspended);
		FOUNDATION_ASSERT(!m_ActiveDialog->IsDone());

		m_ActiveDialog->Cancel();
		m_ActiveDialog->Deactivate(m_Viewport);
		m_ActiveDialog = nullptr;

		Resume();
	}


	bool ComponentsManager::IsNoteInputSilenced() const
	{
		if (IsDisplayingDialog())
			return true;

		if (m_FocusComponent != nullptr)
			return m_FocusComponent->IsNoteInputSilenced();

		return false;
	}


	bool ComponentsManager::IsFastForwardAllowed() const
	{
		if (m_FocusComponent != nullptr)
			return m_FocusComponent->IsFastForwardAllowed();

		return true;
	}


	void ComponentsManager::PullDataFromAllSources(const bool inFromUndo)
	{
		for (auto& component : m_Components)
			component->PullDataFromSource(inFromUndo);
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::AddComponent(std::shared_ptr<ComponentBase> inComponent)
	{
		FOUNDATION_ASSERT(inComponent != nullptr);

		// Get incoming component id
		const int incoming_component_id = inComponent->GetComponentID();

		// Check if the component or a component with the same id is already in the list of components
		for (const auto& component : m_Components)
		{
			FOUNDATION_ASSERT(component != inComponent);
			FOUNDATION_ASSERT(component->GetComponentID() != incoming_component_id);
		}

		m_Components.push_back(inComponent);
		SortComponents();
	}


	void ComponentsManager::RemoveComponent(std::shared_ptr<ComponentBase> inComponent)
	{
		if (&*inComponent == m_FocusComponent)
			SetComponentInFocus(nullptr);

		auto it = m_Components.begin();

		while (it != m_Components.end())
		{
			if (*it == inComponent)
			{
				m_Components.erase(it);
				return;
			}

			++it;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	ComponentBase* ComponentsManager::GetComponent(int inComponentID)
	{
		for (auto& component : m_Components)
		{
			if (component->GetComponentID() == inComponentID)
				return &(*component);
		}

		return nullptr;
	}


	const ComponentBase* ComponentsManager::GetComponent(int inComponentID) const
	{
		for (auto& component : m_Components)
		{
			if (component->GetComponentID() == inComponentID)
				return &(*component);
		}

		return nullptr;
	}


	void ComponentsManager::SetGroupEnabledForInput(unsigned int inGroup, bool inEnable)
	{
		FOUNDATION_ASSERT(inGroup < sizeof(m_EnabledInputGroups) * 8);

		if (inEnable)
			m_EnabledInputGroups |= 1 << inGroup;
		else
			m_EnabledInputGroups &= ~(1 << inGroup);
	}


	bool ComponentsManager::IsGroupEnabledForInput(unsigned int inGroup) const
	{
		FOUNDATION_ASSERT(inGroup < sizeof(m_EnabledInputGroups) * 8);
		return (m_EnabledInputGroups & (1 << inGroup)) != 0;
	}


	void ComponentsManager::SetGroupEnabledForTabbing(unsigned int inGroup)
	{
		FOUNDATION_ASSERT(inGroup < MAX_TAB_GROUP_COUNT);

		if (m_EnabledTabGroup != inGroup && IsGroupEnabledForInput(inGroup))
		{
			m_EnabledTabGroup = inGroup;

			if (m_FocusComponent != nullptr && m_FocusComponent->GetComponentGroupID() != inGroup)
				SetComponentInFocus(m_TabGroupFocusComponent[inGroup]);
		}
	}


	unsigned int ComponentsManager::GetEnabledTabGroup() const
	{
		return m_EnabledTabGroup;
	}


	bool ComponentsManager::IsTabGroupEnabled(unsigned int inGroup) const
	{
		FOUNDATION_ASSERT(inGroup < MAX_TAB_GROUP_COUNT);
		return (m_EnabledTabGroup == inGroup && IsGroupEnabledForInput(inGroup));
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::SetComponentInFocus(int inComponentID)
	{
		ComponentBase* new_focus_component = GetComponent(inComponentID);
		SetComponentInFocus(new_focus_component);
	}


	void ComponentsManager::SetComponentInFocus(std::shared_ptr<ComponentBase> inComponent)
	{
		ComponentBase* new_focus_component = inComponent == nullptr ? nullptr : GetComponent(inComponent->GetComponentID());
		SetComponentInFocus(new_focus_component);
	}


	bool ComponentsManager::IsComponentInFocus(int inComponentID) const
	{
		if (m_FocusComponent != nullptr)
			return m_FocusComponent->GetComponentID() == inComponentID;

		return false;
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::AddVisualizerComponent(std::shared_ptr<VisualizerComponentBase> inComponent)
	{
		m_VisualizerComponents.push_back(inComponent);
	}

	void ComponentsManager::RemoveVisualizerComponent(std::shared_ptr<VisualizerComponentBase> inComponent)
	{
		auto it = m_VisualizerComponents.begin();

		while (it != m_VisualizerComponents.end())
		{
			if (*it == inComponent)
			{
				m_VisualizerComponents.erase(it);
				return;
			}

			++it;
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	bool ComponentsManager::ConsumeInput(const Keyboard& inKeyboard, const Mouse& inMouse)
	{
		if (m_Suspended)
		{
			if (m_ActiveDialog != nullptr)
			{
				const bool dialog_consumed_input = m_ActiveDialog->ConsumeInput(inKeyboard, inMouse);

				if (m_ActiveDialog->IsDone())
				{
					m_ActiveDialog->Deactivate(m_Viewport);
					m_ActiveDialog = nullptr;

					Resume();
				}

				return dialog_consumed_input;
			}
		}
		else
		{
			// Handle tabbing components
			for (auto key_code : inKeyboard.GetKeyEventList())
			{
				unsigned int modifiers = inKeyboard.GetModiferMask();

				if ((modifiers & ~Keyboard::Shift) == 0)
				{
					if (key_code == SDLK_TAB)
					{
						if ((modifiers & Keyboard::Shift) != 0)
						{
							if (m_FocusComponent == nullptr || m_FocusComponent->MayTabOutOfFocus(false))
							{
								SetTabPreviousComponentFocus();

								// Do not process anymore inputs after switching focus by tabbing
								return true;
							}
						}
						else
						{
							if (m_FocusComponent == nullptr || m_FocusComponent->MayTabOutOfFocus(true))
							{
								SetNextTabComponentFocus();

								// Do not process anymore inputs after switching focus by tabbing
								return true;
							}
						}
					}
				}
			}

			// Handle clicking on components
			if (inMouse.IsButtonPressed(Mouse::Left))
			{
				ComponentBase* component = GetComponentAt(inMouse.GetPosition());

				if (component != nullptr && component->CanReceiveFocus())
				{
					if (component != m_FocusComponent)
						SetComponentInFocus(component);

#ifdef _DEBUG
					if (inKeyboard.IsModifierDown(Keyboard::Shift | Keyboard::Control))
						component->GetTextField()->ColorAreaBackground(Color::Red, component->GetRect());
#endif //_DEBUG
				}
			}

			// Consume input on focus component only!
			if (m_FocusComponent != nullptr)
			{
				if (m_FocusComponent->ConsumeInput(inKeyboard, *m_CursorControl, *this))
					return true;
				if (m_FocusComponent->ConsumeInput(inMouse, inKeyboard.GetModiferMask(), *m_CursorControl, *this))
					return true;
			}

			// Consume non exclusive input
			if (m_FocusComponent == nullptr || !m_FocusComponent->ConsumeNonExclusiveInput(inMouse))
			{
				for (auto& component : m_Components)
				{
					if (IsGroupEnabledForInput(component->GetComponentGroupID()))
					{
						if (component->ConsumeNonExclusiveInput(inMouse))
							break;
					}
				}
			}

			// Let visualizer components consume non exclusive mouse input (if the mouse is over them)
			auto* VisualComponent = GetVisualizerComponentAt(inMouse.GetPosition());

			if(VisualComponent != nullptr)
				VisualComponent->ConsumeNonExclusiveInput(inMouse);
		}

		return false;
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::Update(int inDeltaTick, Emulation::CPUMemory* inCPUMemory)
	{
		if (m_Suspended)
		{
			if (m_ActiveDialog != nullptr)
				m_ActiveDialog->Update(inDeltaTick, inCPUMemory);
		}
		else
		{
			// Handle data changes on all components
			if (inCPUMemory != nullptr)
			{
				bool locked = false;

				for (auto& component : m_Components)
				{
					if (component->HasDataChange())
					{
						if (!locked)
						{
							inCPUMemory->Lock();
							locked = true;
						}

						component->HandleDataChange();
					}
				}

				if (locked)
					inCPUMemory->Unlock();
			}
		}
	}

	void ComponentsManager::Refresh(const DisplayState& inDisplayState)
	{
		if (m_Suspended)
		{
			if (m_ActiveDialog != nullptr)
				m_ActiveDialog->Refresh(inDisplayState);
		}
		else
		{
			for (auto& component : m_Components)
				component->Refresh(inDisplayState);

			for (auto& visualizer_component : m_VisualizerComponents)
				visualizer_component->Refresh(inDisplayState);
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::SetComponentInFocus(ComponentBase* inFocusComponent)
	{
		if (m_FocusComponent != inFocusComponent && (inFocusComponent == nullptr || inFocusComponent->CanReceiveFocus()))
		{
			if (m_FocusComponent != nullptr)
				m_FocusComponent->ClearHasControl(*m_CursorControl);

			m_FocusComponent = inFocusComponent;

			if (m_FocusComponent != nullptr)
			{
				unsigned int group_id = inFocusComponent->GetComponentGroupID();

				if (IsGroupEnabledForInput(group_id))
				{
					if (!IsTabGroupEnabled(group_id))
						m_EnabledTabGroup = group_id;

					m_TabGroupFocusComponent[group_id] = inFocusComponent;

					m_FocusComponent->SetHasControl(ComponentBase::GetControlType::Default, *m_CursorControl);

					m_CursorControl->SetTargetTextField(m_FocusComponent != nullptr ? m_FocusComponent->GetTextField() : nullptr);
					m_CursorControl->SetDimensions(CursorControl::Extent({ 1, 1 }));
				}
			}
		}
	}

	void ComponentsManager::SetComponentInFocusByTabbing(ComponentBase* inFocusComponent, bool inWasForwardTab)
	{
		if (m_FocusComponent != inFocusComponent)
		{
			if (m_FocusComponent != nullptr)
				m_FocusComponent->ClearHasControl(*m_CursorControl);

			if (inFocusComponent != nullptr)
				inFocusComponent->SetHasControl(inWasForwardTab ? ComponentBase::GetControlType::Tabbed_Forward : ComponentBase::GetControlType::Tabbed_Backward, *m_CursorControl);

			m_FocusComponent = inFocusComponent;

			if (m_FocusComponent != nullptr)
			{
				unsigned int group_id = inFocusComponent->GetComponentGroupID();
				m_TabGroupFocusComponent[group_id] = inFocusComponent;

				m_CursorControl->SetTargetTextField(m_FocusComponent != nullptr ? m_FocusComponent->GetTextField() : nullptr);
				m_CursorControl->SetDimensions(CursorControl::Extent({ 1, 1 }));
			}
		}
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::SortComponents()
	{
//		std::sort(m_Components.begin(), m_Components.end(), [](std::shared_ptr<ComponentBase> inComponent1, std::shared_ptr<ComponentBase> inComponent2)
//		{
//			return inComponent1->GetComponentID() < inComponent2->GetComponentID();
//		});
	}

	//--------------------------------------------------------------------------------------------------------------------------------

	void ComponentsManager::SetNextTabComponentFocus()
	{
		ComponentBase* next_candidate = GetComponentAfter(m_FocusComponent);

		while (next_candidate != nullptr && next_candidate != m_FocusComponent)
		{
			if (IsTabGroupEnabled(next_candidate->GetComponentGroupID()) && next_candidate->CanReceiveFocus())
			{
				SetComponentInFocusByTabbing(next_candidate, true);
				return;
			}

			next_candidate = GetComponentAfter(next_candidate);
		}
	}


	void ComponentsManager::SetTabPreviousComponentFocus()
	{
		ComponentBase* next_candidate = GetComponentBefore(m_FocusComponent);

		while (next_candidate != nullptr && next_candidate != m_FocusComponent)
		{
			if (IsTabGroupEnabled(next_candidate->GetComponentGroupID())  && next_candidate->CanReceiveFocus())
			{
				SetComponentInFocusByTabbing(next_candidate, false);
				return;
			}

			next_candidate = GetComponentBefore(next_candidate);
		}
	}


	void ComponentsManager::ForceRefresh()
	{
		for (auto& component : m_Components)
			component->ForceRefresh();
	}



	ComponentBase* ComponentsManager::GetComponentAfter(ComponentBase* inComponent) const
	{
		if (!m_Components.empty())
		{
			if (inComponent == nullptr)
				return &*m_Components.front();

			auto it = m_Components.begin();

			while (it != m_Components.end())
			{
				if (&*(*it) == inComponent)
				{
					++it;

					if (it == m_Components.end())
						return &*m_Components.front();
					else
						return &*(*it);
				}

				++it;
			}
		}

		return nullptr;
	}

	ComponentBase* ComponentsManager::GetComponentBefore(ComponentBase* inComponent) const
	{
		if (!m_Components.empty())
		{
			if (inComponent == nullptr)
				return &*m_Components.back();

			auto it = m_Components.rbegin();

			while (it != m_Components.rend())
			{
				if (&*(*it) == inComponent)
				{
					++it;

					if (it == m_Components.rend())
						return &*m_Components.back();
					else
						return &*(*it);
				}

				++it;
			}
		}

		return nullptr;
	}


	ComponentBase* ComponentsManager::GetComponentAt(const Point& inPosition) const
	{
		for (auto component : m_Components)
		{
			if (IsGroupEnabledForInput(component->GetComponentGroupID()) && component->ContainsPosition(inPosition))
				return &*component;
		}

		return nullptr;
	}


	VisualizerComponentBase* ComponentsManager::GetVisualizerComponentAt(const Foundation::Point& inPosition) const
	{
		for (auto visualizerComponent : m_VisualizerComponents)
		{
			if (visualizerComponent->ContainsPosition(inPosition))
				return &*visualizerComponent;
		}

		return nullptr;
	}

}