#if EDITOR && ENGINE_CSHARP
#pragma once
#include "../EditorPanel.h"
#include <UI/UIScrollBox.h>

class CSharpErrorList : public EditorPanel
{
public:
	CSharpErrorList();

	void Tick() override;

	enum class ErrorLevel
	{
		Message,
		Warning,
		Error
	};

	struct Message
	{
		std::string Code;
		std::string Message;
		std::string File;
		size_t Line;
		ErrorLevel Level;
	};


	void AddMessage(Message NewMessage);

	void ClearMessages();
private:
	void Generate();
	UIScrollBox* MessageScrollBox = new UIScrollBox(UIBox::Orientation::Vertical, 0, true);
	static std::vector<Message> Messages;
	static std::vector<Message> NewMessages;

};

#endif