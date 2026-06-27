# Tetriz

一个 C++20 终端版俄罗斯方块。

## 功能

- 10x20 棋盘、7-bag 随机方块、Next 预览
- Hold、Ghost 影子方块、软降、硬降、旋转和简化墙踢
- 自动下落、落地锁定、消行、计分、等级加速
- 暂停、重开、Game Over、退出后恢复终端光标和颜色

## 按键

| Key | Action |
| --- | --- |
| `A` / `D` | 左右移动 |
| `S` | 软降 |
| `W` | 顺时针旋转 |
| `Space` | 硬降 |
| `C` | Hold |
| `P` | 暂停/继续 |
| `R` | 重开 |
| `Q` | 退出 |

## 构建和运行

需要支持 C++20 的编译器和 CMake。

macOS 如果没有编译工具，先安装：

```sh
xcode-select --install
```

构建并运行：

```sh
cmake -S . -B build
cmake --build build
./build/tetriz
```

## 参考

- ANSI Escape Sequences: https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797
- Unicode Box: https://en.wikipedia.org/wiki/Box-drawing_characters
- SRS: https://harddrop.com/wiki/SRS
