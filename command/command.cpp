#include "command.h"
#include <sstream>
#include <algorithm>

using namespace NSCommand;

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

void CommandLib::Init(IFont* font, ISoundEffect* pSE, ISprite* sprCursor)
{
    m_font = font;
    m_SE = pSE;
    m_sprCursor = sprCursor;
}

void NSCommand::CommandLib::UpsertCommand(const std::string& name, const bool enable)
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

    ResetRect();
}

void NSCommand::CommandLib::RemoveCommand(const std::string& name)
{
    auto result = std::remove_if(m_commandList.begin(), m_commandList.end(),
                                 [&](const Command& x)
                                 {
                                     return x.GetName() == name;
                                 });

    m_commandList.erase(result, m_commandList.end());

    ResetRect();
}

void CommandLib::Draw()
{
    // コマンドを中央揃えで表示する
    for (int i = 0; i < (int)m_commandList.size(); ++i)
    {
        bool enable = m_commandList.at(i).GetEnable();
        // 選択可能コマンド
        if (enable)
        {
            // 選択中コマンド
            if (m_cursorIndex == i)
            {
                m_font->DrawText_(m_commandList.at(i).GetName(),
                                  m_commandList.at(i).GetLeftPos(),
                                  STARTY,
                                  255);
            }
            // 未選択コマンド
            else
            {
                m_font->DrawText_(m_commandList.at(i).GetName(),
                                  m_commandList.at(i).GetLeftPos(),
                                  STARTY,
                                  128);
            }
        }
        // 使用不可コマンド
        else
        {
            m_font->DrawText_(m_commandList.at(i).GetName(),
                              m_commandList.at(i).GetLeftPos(),
                              STARTY,
                              64);
        }

        // カーソルの表示
        if (m_cursorIndex == i)
        {
            int x = 0;
            x = m_commandList.at(i).GetLeftPos();
            // 100/2ピクセル右にずらす。丸の半径が10ピクセルくらいなので少し左に戻す。
            x += (COMMAND_WIDTH / 2) - 5;
            m_sprCursor->DrawImage(x, STARTY + CURSOR_PADDING_Y);
        }
    }
}

void NSCommand::CommandLib::Previous()
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
        --m_cursorIndex;
        if (m_cursorIndex <= -1)
        {
            m_cursorIndex = m_commandList.size() - 1;
        }

        if (m_commandList.at(m_cursorIndex).GetEnable() == false)
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

void NSCommand::CommandLib::Next()
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
        ++m_cursorIndex;
        if (m_cursorIndex >= (int)m_commandList.size())
        {
            m_cursorIndex = 0;
        }

        if (m_commandList.at(m_cursorIndex).GetEnable() == false)
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

std::string NSCommand::CommandLib::Into()
{
    m_SE->PlayClick();
    return m_commandList.at(m_cursorIndex).GetName();
}

void NSCommand::CommandLib::MouseMove(const int x, const int y)
{
    int index = -1;
    for (int i = 0; i < (int)m_commandList.size(); ++i)
    {
        int top = 0;
        int left = 0;
        int bottom = 0;
        int right = 0;
        m_commandList.at(i).GetRect(&top, &left, &bottom, &right);

        if (left <= x && x <= right)
        {
            if (top <= y && y <= bottom)
            {
                index = i;
                break;
            }
        }
    }
    if (index != -1 && m_cursorIndex != index)
    {
        if (m_commandList.at(index).GetEnable())
        {
            m_cursorIndex = index;
            m_SE->PlayMove();
        }
    }
}

std::string NSCommand::CommandLib::Click(const int x, const int y)
{
    int index = -1;
    for (int i = 0; i < (int)m_commandList.size(); ++i)
    {
        int top = 0;
        int left = 0;
        int bottom = 0;
        int right = 0;
        m_commandList.at(i).GetRect(&top, &left, &bottom, &right);

        if (left <= x && x <= right)
        {
            if (top <= y && y <= bottom)
            {
                index = i;
                break;
            }
        }
    }

    if (index != -1)
    {
        if (m_commandList.at(index).GetEnable())
        {
            m_cursorIndex = index;
            m_SE->PlayClick();
        }
    }

    if (index == -1)
    {
        return "";
    }
    else
    {
        return m_commandList.at(m_cursorIndex).GetName();
    }
}

void NSCommand::CommandLib::ResetRect()
{
    // 奇数の場合
    if (m_commandList.size() % 2 == 1)
    {
        for (int i = 0; i < (int)m_commandList.size(); ++i)
        {
            int top = 0;
            int left = 0;
            int bottom = 0;
            int right = 0;

            top = STARTY;
            bottom = STARTY + COMMAND_HEIGHT;

            left = CENTERX;
            left += INTERVAL * (i - (m_commandList.size() / 2));
            right = left + COMMAND_WIDTH;

            m_commandList.at(i).SetRect(top, left, bottom, right);
        }
    }
    // 偶数の場合
    else
    {
        for (int i = 0; i < (int)m_commandList.size(); ++i)
        {
            int top = 0;
            int left = 0;
            int bottom = 0;
            int right = 0;

            top = STARTY;
            bottom = STARTY + COMMAND_HEIGHT;

            left = CENTERX;
            left += INTERVAL * (i - (m_commandList.size() / 2));
            left += INTERVAL / 2;
            right = left + COMMAND_WIDTH;

            m_commandList.at(i).SetRect(top, left, bottom, right);
        }
    }
}

void NSCommand::Command::SetName(const std::string& arg)
{
    m_name = arg;
}

std::string NSCommand::Command::GetName() const
{
    return m_name;
}

void NSCommand::Command::SetEnable(const bool arg)
{
    m_bEnable = arg;
}

bool NSCommand::Command::GetEnable() const
{
    return m_bEnable;
}

void NSCommand::Command::SetRect(const int top, const int left, const int bottom, const int right)
{
    m_top = top;
    m_left = left;
    m_bottom = bottom;
    m_right = right;
}

void NSCommand::Command::GetRect(int* top, int* left, int* bottom, int* right)
{
    *top = m_top;
    *left = m_left;
    *bottom = m_bottom;
    *right = m_right;
}

int NSCommand::Command::GetLeftPos()
{
    return m_left;
}
