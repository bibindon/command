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

void hud::Init(IFont* font, ISprite* sprBack, ISprite* sprMiddle, ISprite* sprFront)
{
    m_font = font;
    m_sprBack = sprBack;
    m_sprMiddle = sprMiddle;
    m_sprFront = sprFront;
}

void NSHud::hud::UpsertStatus(const std::string& name,
                                        const int percent,
                                        const int percentSub,
                                        const bool visible)
{
    auto result = std::find_if(m_statusList.begin(), m_statusList.end(),
                               [&](const StatusItem& x)
                               {
                                   return x.GetName() == name;
                               });

    if (result != m_statusList.end())
    {
        result->SetPercent(percent);
        result->SetPercentSub(percentSub);
        result->SetBarVisible(visible);
    }
    else
    {
        StatusItem statusItem;
        statusItem.SetName(name);
        statusItem.SetPercent(percent);
        statusItem.SetPercentSub(percentSub);
        statusItem.SetBarVisible(visible);

        m_statusList.push_back(statusItem);
    }
}

void NSHud::hud::RemoveStatus(const std::string& name)
{
    auto result = std::remove_if(m_statusList.begin(), m_statusList.end(),
                                 [&](const StatusItem& x)
                                 {
                                     return x.GetName() == name;
                                 });

    m_statusList.erase(result, m_statusList.end());
}

void hud::Draw()
{
    // どれだけステータス異常があっても表示できるのは8行までとする？
    for (std::size_t i = 0; i < 8; ++i)
    {
        if (m_statusList.size() <= i)
        {
            break;
        }

        m_font->DrawText_(m_statusList.at(i).GetName(),
                          STARTX + 10,
                          STARTY + (INTERVAL * (int)i));

        if (m_statusList.at(i).GetBarVisible())
        {
            m_sprBack->DrawImage(100,
                                 STARTX,
                                 PADDING + STARTY + (INTERVAL * (int)i));

            m_sprMiddle->DrawImage(m_statusList.at(i).GetPercentSub(),
                                   STARTX,
                                   PADDING + STARTY + (INTERVAL * (int)i));

            m_sprFront->DrawImage(m_statusList.at(i).GetPercent(),
                                  STARTX,
                                  PADDING + STARTY + (INTERVAL * (int)i));
        }
    }
}

void NSHud::StatusItem::SetName(const std::string& arg)
{
    m_name = arg;
}

std::string NSHud::StatusItem::GetName() const
{
    return m_name;
}

void NSHud::StatusItem::SetPercent(const int arg)
{
    m_percent = arg;
}

int NSHud::StatusItem::GetPercent() const
{
    return m_percent;
}

void NSHud::StatusItem::SetPercentSub(const int arg)
{
    m_percentSub = arg;
}

int NSHud::StatusItem::GetPercentSub() const
{
    return m_percentSub;
}

void NSHud::StatusItem::SetBarVisible(const bool arg)
{
    m_visible = arg;
}

bool NSHud::StatusItem::GetBarVisible() const
{
    return m_visible;
}
