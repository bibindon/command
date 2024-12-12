// ��ʂɃX�e�[�^�X����\������B
// ���b�N�}���̗̑̓o�[�̂悤�Ȃ���
// hud�Ƃ������O�͗ǂ��Ȃ������C�����邪�ς����Ȃ��̂ł��̂܂܂Ƃ���

#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// �A�C�e�������E��2��ŕ\������
// �����C���x���g���A�E���q��
// �N���b�N������E�����Ɉړ�����B�m�F�͍s��Ȃ��B
namespace NSHud
{
class ISprite
{
public:
    virtual void DrawImage(const int x,
                           const int y,
                           const int transparency = 255) = 0;

    virtual void Load(const std::string& filepath) = 0;
    virtual ~ISprite() {};
};

class IFont
{
public:
    virtual void DrawText_(const std::string& msg,
                           const int x,
                           const int y,
                           const int transparent) = 0;

    virtual void Init() = 0;
    virtual ~IFont() {};
};

class ISoundEffect
{
public:
    virtual void PlayMove() = 0;
    virtual void PlayClick() = 0;
    virtual void PlayBack() = 0;
    virtual void Init() = 0;
    virtual ~ISoundEffect() {};
};


class Command
{
public:
    void SetName(const std::string& arg);
    std::string GetName() const;

    void SetEnable(const bool arg);
    bool GetEnable() const;

private:
    std::string m_name;
    bool m_bEnable = false;
};

class hud
{
public:

    void Init(IFont* font, ISoundEffect* SE, ISprite* sprCursor);

    void UpsertCommand(const std::string& name,
                       const bool enable);

    void RemoveCommand(const std::string& name);

    void Draw();

    void Previous();
    void Next();
    std::string Into();
    
private:

    ISprite* m_sprCursor;
    IFont* m_font;
    ISoundEffect* m_SE;

    std::vector<Command> m_commandList;

    const int STARTX = 750;
    const int STARTY = 850;

    const int INTERVAL = 100;
    const int CURSOR_PADDING_Y = -25;

    int m_cursor = 0;

};
}

