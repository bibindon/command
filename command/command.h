// 画面にステータス情報を表示する。
// ロックマンの体力バーのようなもの
// hudという名前は良くなかった気がするが変えられないのでそのままとする

#pragma once
#include <string>
#include <vector>
#include <unordered_map>

// アイテムを左右の2列で表示する
// 左がインベントリ、右が倉庫
// クリックしたら右か左に移動する。確認は行わない。
namespace NSCommand
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

    void SetRect(const int top, const int left, const int bottom, const int right);
    void GetRect(int* top, int* left, int* bottom, int* right);
    int GetLeftPos();

private:
    std::string m_name;
    bool m_bEnable = false;

    int m_top = 0;
    int m_left = 0;
    int m_bottom = 0;
    int m_right = 0;
};

class CommandLib
{
public:

    void Init(IFont* font, ISoundEffect* SE, ISprite* sprCursor);
    void Finalize();

    void UpsertCommand(const std::string& name,
                       const bool enable);

    void RemoveCommand(const std::string& name);

    void RemoveAll();

    void Draw();

    void Previous();
    void Next();
    std::string Into();
    void MouseMove(const int x, const int y);
    std::string Click(const int x, const int y);
    
private:

    void ResetRect();

    ISprite* m_sprCursor;
    IFont* m_font;
    ISoundEffect* m_SE;

    std::vector<Command> m_commandList;

    const int CENTERX = 750;
    const int STARTY = 800;

    const int INTERVAL = 100;
    const int CURSOR_PADDING_Y = 16;

    int m_cursorIndex = 0;
    const int COMMAND_WIDTH = 100;
    const int COMMAND_HEIGHT = 100;

};
}

