# Tetriz

Tetriz 是一个使用 C++20 编写的终端版俄罗斯方块。项目直接在命令行里绘制游戏界面，包含完整的游戏循环、键盘输入、方块生成、消行计分和终端渲染逻辑。

## 项目功能

- 经典 10x20 俄罗斯方块棋盘
- 7-bag 随机方块生成，保证每组 7 个方块各出现一次
- Next 队列预览，提前展示后续方块
- Hold 功能，可以暂存当前方块并在后续交换使用
- Ghost 影子方块，显示当前方块硬降后的落点
- 自动下落、软降、硬降、顺/逆时针旋转、180 度旋转和简化墙踢
- 方块落地锁定、满行清除、分数计算、Combo、Back-to-back 和等级加速
- 本地最高分保存和状态栏展示
- 暂停、继续、重开和 Game Over 状态
- 退出或 Ctrl+C 时恢复终端光标和颜色
- 使用局部刷新和 ASCII 边框，减少终端闪烁和字符错位问题

## 游戏界面

游戏运行后会在终端中显示以下区域：

- `Hold`：暂存方块
- `Tetriz`：主游戏棋盘
- `Status`：分数、最高分、等级、消行数、Combo、Back-to-back、FPS 和当前状态
- `High`：本地最高分，默认保存到当前目录的 `.tetriz_high_score`
- `Next`：后续方块预览
- `Controls`：按键提示

建议终端窗口至少保持 `70 列 x 22 行`。窗口太小时游戏会显示尺寸提示，恢复到足够尺寸后自动重绘。

## 按键操作

| Key | Action |
| --- | --- |
| `A` / `D`, `←` / `→` | 左右移动 |
| `S`, `↓` | 软降 |
| `W`, `↑` | 顺时针旋转 |
| `Z` | 逆时针旋转 |
| `X` | 180 度旋转 |
| `Space` | 硬降 |
| `C` | Hold |
| `P` | 暂停/继续 |
| `R` | 重开 |
| `Q`, `Ctrl+C` | 退出 |

按键大小写都可以使用。

## 构建和运行

需要安装支持 C++20 的编译器和 CMake。

macOS 如果没有编译工具，先安装 Xcode Command Line Tools：

```sh
xcode-select --install
```

构建：

```sh
cmake -S . -B build
cmake --build build
```

运行：

```sh
./build/tetriz
```

高分默认保存到当前目录的 `.tetriz_high_score`。可以通过环境变量改保存路径：

```sh
TETRIZ_HIGH_SCORE_PATH=/tmp/tetriz_high_score ./build/tetriz
```

测试：

```sh
ctest --test-dir build --output-on-failure
```

## 代码结构

- `main.cpp`：程序入口和主循环
- `game.cpp` / `game.h`：游戏状态、方块逻辑、碰撞、消行、计分和随机队列
- `draw.cpp` / `draw.h`：终端 UI 绘制和局部刷新
- `control.cpp` / `control.h`：键盘监听和操作映射
- `terminal.cpp` / `terminal.h`：终端控制、光标、颜色和输入模式封装
- `utils.cpp` / `utils.h`：FPS 统计和辅助函数
- `define.h` / `color.h`：全局常量和颜色定义

## 实现说明

项目将游戏逻辑和渲染拆开：游戏层维护棋盘、当前方块、Hold、Next 队列、分数和状态；渲染层只读取游戏快照并更新终端界面。输入线程负责监听按键并调用游戏动作，主循环按固定节奏驱动自动下落和画面刷新。

当前旋转规则使用简化墙踢，重点是保证终端版游戏稳定可玩；如果要进一步接近现代俄罗斯方块规则，可以继续扩展为完整 SRS。

## 参考

- ANSI Escape Sequences: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
- Unicode Box: https://en.wikipedia.org/wiki/Box-drawing_characters
- SRS: https://harddrop.com/wiki/SRS
