#if EDITOR && ENGINE_CSHARP
#include "CSharpErrorList.h"
#include <UI/UIText.h>
#include <UI/EditorUI/EditorUI.h>
#include <mutex>
#include <Engine/Utility/FileUtility.h>

static std::mutex MessageMutex;

std::vector<CSharpErrorList::Message> CSharpErrorList::Messages;
std::vector<CSharpErrorList::Message> CSharpErrorList::NewMessages;

CSharpErrorList::CSharpErrorList()
	: EditorPanel(0, 0, "C# Error List", "csharp_errors")
{
	CanBeClosed = true;

	PanelMainBackground->AddChild(MessageScrollBox
		->SetTryFill(true)
		->SetPadding(0.005f)
		->SetPaddingSizeMode(UIBox::SizeMode::AspectRelative));
}

void CSharpErrorList::Tick()
{
	{
		std::lock_guard Guard{ MessageMutex };
		
		if (!NewMessages.size())
		{
			return;
		}

		for (const auto& i : NewMessages)
		{
			Messages.push_back(i);
		}
		NewMessages.clear();
	}

	Generate();
}

void CSharpErrorList::AddMessage(Message NewMessage)
{
	std::lock_guard Guard{ MessageMutex };
	NewMessages.push_back(NewMessage);
}

void CSharpErrorList::ClearMessages()
{
}

void CSharpErrorList::Generate()
{
	MessageScrollBox->DeleteChildren();
	for (const auto& i : Messages)
	{
		UIBox* MessageEntry = new UIBox(UIBox::Orientation::Horizontal, 0);

		MessageScrollBox->AddChild(MessageEntry
			->AddChild(new UIText(0.45f, 1, i.Code, EditorUI::Text))
			->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, 0.75f, 1))
				->SetTryFill(true)
				->SetPadding(0, 0, 0.01f, 0.01f)
				->SetSizeMode(UIBox::SizeMode::PixelRelative))
			->AddChild(new UIText(0.45f, 1, i.Message, EditorUI::Text))
			->AddChild((new UIBackground(UIBox::Orientation::Horizontal, 0, 0.75f, 1))
				->SetTryFill(true)
				->SetPadding(0, 0, 0.01f, 0.01f)
				->SetSizeMode(UIBox::SizeMode::PixelRelative))
			->AddChild(new UIText(0.45f, 1, FileUtil::GetFileNameFromPath(i.File), EditorUI::Text))
		);
	}
}

#endif