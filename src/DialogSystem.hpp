#pragma once
#include "Tako.hpp"
#include "OpenGLPixelArtDrawer.hpp"
#include "Font.hpp"
#include "Renderer.hpp"

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
			auto fullText = s.m_dialog.size() == s.m_displayed.size();
			if (fullText && input->GetKeyDown(tako::Key::L) || input->GetKeyDown(tako::Key::Gamepad_A) && s.closeDown <= 0)
			{
				s.m_dialog = "";
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

	static void StartDialog(std::string str)
	{
		auto& s = Instance();
		s.m_dialog = str;
		LOG("{}", s.m_dialog);
		s.charDisp = 1;
		s.charCountDown = s.m_charDelay;
		s.UpdateText(str.substr(0, 1));
	}

	static bool IsOpen()
	{
		auto& s = Instance();
		return s.m_dialog.size() > 0 || s.closeDown > 0;
	}

private:
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
	float closeDown = 0;
	float m_charDelay = 0.075f;
	tako::OpenGLPixelArtDrawer* m_drawer;
};
