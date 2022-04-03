#pragma once
#include "Tako.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#include "Font.hpp"
#include "Renderer.hpp"
#include <variant>
#include "Routine.hpp"

using DialogCallback = std::function<void()>;
using BoolReturn = std::function<bool()>;
using DialogCallbackWait = std::function<BoolReturn()>;
using DialogPart = std::variant<std::string, DialogCallback, DialogCallbackWait>;

class Dialog
{
public:
	Dialog(std::vector<DialogPart>&& parts)
	{
		m_parts = std::move(parts);
	}

	template<typename... Ps>
	Dialog(Ps&&... parts)
	{
		(m_parts.push_back(parts),...);
	}

	DialogPart& GetPart(int index)
	{
		return m_parts[index];
	}

	int GetPartCount()
	{
		return m_parts.size();
	}
private:
	std::vector<DialogPart> m_parts;
};

class DialogSystem
{
public:
	static void Init(tako::OpenGLPixelArtDrawer* drawer)
	{
		auto& s = Instance();
		s.m_drawer = drawer;
		auto bitmap = tako::Bitmap(1,1);
		s.m_texture = s.m_drawer->CreateTexture(bitmap);
		s.m_nightTexture = s.m_drawer->CreateTexture(bitmap);
		s.m_arrow = s.m_drawer->CreateTexture(tako::Bitmap::FromFile("/ChatArrow.png"));
		s.m_font = new tako::Font("/charmap-cellphone.png", 5, 7, 1, 1, 2, 2,
			" !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]\a_`abcdefghijklmnopqrstuvwxyz{|}~");
	}

	static void Update(tako::Input* input, float dt)
	{
		auto& s = Instance();
		if (IsOpen())
		{
			auto fullText = s.m_dialog.size() <= s.m_displayed.size();
			if (fullText && input->GetKeyDown(tako::Key::L) || input->GetKeyDown(tako::Key::Gamepad_A) && s.closeDown <= 0)
			{
				s.StartDialogPart(s.dialogPart + 1);
				s.closeDown = s.m_charDelay;
			}
			else if (s.closeDown <= 0)
			{
				if (s.charCountDown <= 0 && s.charDisp < s.m_dialog.size())
				{
					s.charDisp++;
					s.charCountDown = s.charDisp == s.m_dialog.size() ? s.m_charDelay * 6 : s.m_charDelay;
					s.UpdateText(s.m_dialog.substr(0, s.charDisp));
				}
				s.charCountDown -= dt;
			}
			s.closeDown -= dt;
		}
	}

	static void Draw(tako::OpenGLPixelArtDrawer* drawer, const Palette& palette)
	{
		auto& s = Instance();

		if (s.m_showNightText)
		{
			drawer->SetTargetSize(240, 135);
			drawer->SetCameraPosition({0, 0});
			auto texX = s.m_nightTexture.width;
			auto texY = s.m_nightTexture.height;
			drawer->DrawImage(0-texX/2, 0+texY/2, texX, texY, s.m_nightTexture.handle);
		}

		if (!IsOpen())
		{
			return;
		}

		drawer->SetTargetSize(240, 135);
		float winX = 190;
		float winY = 135/5;
		drawer->SetCameraPosition({winX/2, winY});
		//auto cameraSize = drawer->GetCameraViewSize();
		drawer->DrawRectangle(1, 1, winX-2, winY-1, palette[0]);
		drawer->DrawRectangle(2, 0, winX-4, winY-3, palette[3]);
		drawer->DrawImage(4, -2, s.m_texture.width, s.m_texture.height, s.m_texture.handle, palette[1]);
		if (s.charCountDown < 0 && s.m_displayed.size() == s.m_dialog.size())
		{
			drawer->DrawImage(winX - 8, -winY+10, 3, 5, s.m_arrow.handle, palette[0]);
		}

		drawer->SetTargetSize(240/2, 135/2);
	}

	static void StartDialog(Dialog&& dialog)
	{
		auto& s = Instance();
		s.m_activeDialog = std::move(dialog);
		s.StartDialogPart(0);
	}

	static bool IsOpen()
	{
		auto& s = Instance();
		return s.m_waitCallback || s.m_dialog.size() > 0 || s.closeDown > 0 || s.m_activeDialog.GetPartCount() > s.dialogPart;
	}

	static void NightTimeReading(std::vector<const char*> text, std::optional<std::function<void()>> finishCallback)
	{
		static float ct;
		static int textIndex;
		static int charIndex;
		ct = 0.3f;
		textIndex = 0;
		charIndex = 0;
		Instance().RenderNightText(text[textIndex], charIndex);
		Routine::Register([=](auto input, float dt)
		{
			ct -= dt;
			PaletteManager::Black();
			auto& s = Instance();
			if (ct <= 0)
			{
				if (textIndex < text.size())
				{
					auto len = strlen(text[textIndex]);
					if (charIndex < len)
					{
						charIndex++;
						s.RenderNightText(text[textIndex], charIndex);
						ct = charIndex == len ? 1.0f : 0.3f;
					}
					else
					{
						textIndex++;
						charIndex = 0;
						if (textIndex < text.size())
						{
							s.RenderNightText(text[textIndex], charIndex);
						}
						else
						{
							s.ClearNightText();
						}
						ct = 1.0f;
					}
				}
				else
				{
					if (finishCallback)
					{
						finishCallback.value()();
					}
					return false;
				}
			}

			return true;
		});
	}

private:
	void StartDialogPart(int index)
	{
		dialogPart = index;
		m_dialog = "";
		if (index >= m_activeDialog.GetPartCount())
		{
			return;
		}
		auto& part = m_activeDialog.GetPart(index);
		if (std::holds_alternative<DialogCallback>(part))
		{
			std::get<DialogCallback>(part)();
			StartDialogPart(index+1);
			return;
		}
		if (std::holds_alternative<DialogCallbackWait>(part))
		{
			m_waitCallback = std::get<DialogCallbackWait>(part)();
			return;
		}
		auto str = std::get<std::string>(part);
		m_dialog = str;
		LOG("{}", m_dialog);
		charDisp = 1;
		charCountDown = m_charDelay;
		UpdateText(str.substr(0, 1));
	}

	void UpdateText(std::string s)
	{
		if (m_displayed == s)
		{
			return;
		}
		auto bitmap = m_font->RenderText(s, 1);
		m_displayed = s;
		m_drawer->UpdateTexture(m_texture, bitmap);
	}

	void RenderNightText(const char* fullText, int chars)
	{
		LOG("{}", std::string_view(fullText, chars));
		if (chars <= 0)
		{
			m_showNightText = false;
			return;
		}

		auto bitmap = m_font->RenderText(std::string_view(fullText, chars), 1);
		m_drawer->UpdateTexture(m_nightTexture, bitmap);
		m_showNightText = true;
	}

	void ClearNightText()
	{
		LOG("");
		m_showNightText = false;
	}

	DialogSystem() {}
	static DialogSystem& Instance();
	std::string m_dialog;
	std::string m_displayed;
	int charDisp = 0;
	float charCountDown = 0;
	tako::Texture m_texture;
	tako::Texture m_arrow;
	tako::Font* m_font;
	tako::Texture m_nightTexture;
	bool m_showNightText = false;
	Dialog m_activeDialog;
	std::optional<std::function<bool()>> m_waitCallback;
	int dialogPart = 0;
	float closeDown = 0;
	float m_charDelay = 0.075f;
	tako::OpenGLPixelArtDrawer* m_drawer;
};
