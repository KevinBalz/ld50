#pragma once
#include "Tako.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#include "Font.hpp"
#include "Renderer.hpp"
#include <variant>

using DialogCallback = std::function<void()>;
using DialogPart = std::variant<std::string, DialogCallback>;

class Dialog
{
public:
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
		if (!IsOpen())
		{
			return;
		}

		drawer->SetTargetSize(240, 135);
		float winX = 240;
		float winY = 135/4;
		drawer->SetCameraPosition({winX/2, winY});
		//auto cameraSize = drawer->GetCameraViewSize();
		drawer->DrawRectangle(1, 1, winX-2, winY-1, palette[0]);
		drawer->DrawRectangle(2, 0, winX-4, winY-3, palette[3]);
		drawer->DrawImage(3, -1, s.m_texture.width, s.m_texture.height, s.m_texture.handle, palette[1]);
		if (s.charCountDown < 0 && s.m_displayed.size() == s.m_dialog.size())
		{
			drawer->DrawImage(winX - 8, -winY/2-5, 3, 5, s.m_arrow.handle, palette[0]);
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
		return s.m_dialog.size() > 0 || s.closeDown > 0 || s.m_activeDialog.GetPartCount() > s.dialogPart;
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

	DialogSystem() {}
	static DialogSystem& Instance();
	std::string m_dialog;
	std::string m_displayed;
	int charDisp = 0;
	float charCountDown = 0;
	tako::Texture m_texture;
	tako::Texture m_arrow;
	tako::Font* m_font;
	Dialog m_activeDialog;
	int dialogPart = 0;
	float closeDown = 0;
	float m_charDelay = 0.075f;
	tako::OpenGLPixelArtDrawer* m_drawer;
};