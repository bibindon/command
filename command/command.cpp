#include "command.h"
#include <sstream>
#include <algorithm>

using namespace NSHud;

static std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;

    while (getline(ss, item, delim))
    {
        result.push_back(item);
    }

    return result;
}

void hud::Init(IFont* font, ISoundEffect* pSE, ISprite* sprCursor)
{
    m_font = font;
    m_SE = pSE;
    m_sprCursor = sprCursor;
}

void NSHud::hud::UpsertCommand(const std::string& name, const bool enable)
{
    auto result = std::find_if(m_commandList.begin(), m_commandList.end(),
                               [&](const Command& x)
                               {
                                   return x.GetName() == name;
                               });

    if (result != m_commandList.end())
    {
        result->SetEnable(enable);
    }
    else
    {
        Command command;
        command.SetName(name);
        command.SetEnable(enable);

        m_commandList.push_back(command);
    }
}

void NSHud::hud::RemoveCommand(const std::string& name)
{
    auto result = std::remove_if(m_commandList.begin(), m_commandList.end(),
                                 [&](const Command& x)
                                 {
                                     return x.GetName() == name;
                                 });

    m_commandList.erase(result, m_commandList.end());
}

void hud::Draw()
{
    // コマンドを中央揃えで表示する
    // 奇数の時
    if (m_commandList.size() % 2 == 1)
    {
        int halfListSize = m_commandList.size() / 2;
        for (std::size_t i = 0; i < m_commandList.size(); ++i)
        {
            int i_ = (int)i;

            bool enable = m_commandList.at(i).GetEnable();
            if (enable)
            {
                if (m_cursor == i_)
                {
                    m_font->DrawText_(m_commandList.at(i).GetName(),
                                      STARTX + (INTERVAL * (i_ - halfListSize)),
                                      STARTY,
                                      255);
                }
                else
                {
                    m_font->DrawText_(m_commandList.at(i).GetName(),
                                      STARTX + (INTERVAL * (i_ - halfListSize)),
                                      STARTY,
                                      128);
                }
            }
            else
            {
                m_font->DrawText_(m_commandList.at(i).GetName(),
                                  STARTX + (INTERVAL * (i_ - halfListSize)),
                                  STARTY,
                                  64);
            }

            if (m_cursor == i_)
            {
                int x = 0;
                x += STARTX;
                x += INTERVAL * (i_ - halfListSize);
                // 文字数の分カーソルの位置をずらす。
                x += (m_commandList.at(i).GetName().size() / 2 - 1) * 10;
                m_sprCursor->DrawImage(x,
                                       STARTY + CURSOR_PADDING_Y);
            }
        }
    }
    else
    {
        int halfListSize = m_commandList.size() / 2;
        for (std::size_t i = 0; i < m_commandList.size(); ++i)
        {
            int i_ = (int)i;

            bool enable = m_commandList.at(i).GetEnable();
            if (enable)
            {
                m_font->DrawText_(m_commandList.at(i).GetName(),
                                  STARTX + (INTERVAL * (i_ - halfListSize)) + (INTERVAL / 2),
                                  STARTY,
                                  255);
            }
            else
            {
                m_font->DrawText_(m_commandList.at(i).GetName(),
                                  STARTX + (INTERVAL * (i_ - halfListSize)) + (INTERVAL / 2),
                                  STARTY,
                                  127);
            }
        }
    }
}

void NSHud::hud::Previous()
{
    auto it = std::find_if(m_commandList.begin(), m_commandList.end(),
                           [](const Command& x)
                           {
                               return x.GetEnable();
                           });
    if (it == m_commandList.end())
    {
        return;
    }

    while (true)
    {
        --m_cursor;
        if (m_cursor <= -1)
        {
            m_cursor = m_commandList.size() - 1;
        }

        if (m_commandList.at(m_cursor).GetEnable() == false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    m_SE->PlayMove();
}

void NSHud::hud::Next()
{
    auto it = std::find_if(m_commandList.begin(), m_commandList.end(),
                           [](const Command& x)
                           {
                               return x.GetEnable();
                           });
    if (it == m_commandList.end())
    {
        return;
    }

    while (true)
    {
        ++m_cursor;
        if (m_cursor >= (int)m_commandList.size())
        {
            m_cursor = 0;
        }

        if (m_commandList.at(m_cursor).GetEnable() == false)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    m_SE->PlayMove();
}

std::string NSHud::hud::Into()
{
    m_SE->PlayClick();
    return m_commandList.at(m_cursor).GetName();
}

void NSHud::Command::SetName(const std::string& arg)
{
    m_name = arg;
}

std::string NSHud::Command::GetName() const
{
    return m_name;
}

void NSHud::Command::SetEnable(const bool arg)
{
    m_bEnable = arg;
}

bool NSHud::Command::GetEnable() const
{
    return m_bEnable;
}
