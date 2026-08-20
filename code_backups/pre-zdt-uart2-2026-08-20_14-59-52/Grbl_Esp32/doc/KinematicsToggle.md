说明: 运行时在固件中切换运动学

- G800: 切换到 CoreXY 运动学（需要 CoreXY kinematics 文件，例如 `Custom/CoreXY.cpp`）
- G801: 切换到 Cartesian（普通笛卡尔）运动学
 - G802: 切换到 SwapXY（交换 X 与 Y 轴）

设置替代：`$K0` 现在为整数模式开关：`$K0=0` = Cartesian，`$K0=1` = CoreXY，`$K0=2` = SwapXY。

如何测试:
1. 连接到控制器的串口或 Web UI
2. 发送 `G800`，控制器应回复 `Kinematics set to CoreXY`。
3. 发送 G-code 运动（例如 `G0 X10 Y10`），观察机器是否按 CoreXY kinematics 行动。
4. 发送 `G801`，控制器应回复 `Kinematics set to Cartesian`，随后运动应按普通 Cartesian 行为执行。
5. 发送 `G802`，控制器应回复 `Kinematics set to SwapXY (X<->Y)`，随后 X 与 Y 将在运动映射时互换。

实现细节:
- 新增 `src/Kinematics.h`/`src/Kinematics.cpp` 管理当前运动学模式
- 在 `Custom/CoreXY.cpp` 的 `cartesian_to_motors` 中检测当前模式，若为 Cartesian 则直接通过 `mc_line()` 提交原始笛卡尔目标
- 在 `src/GCode.cpp` 中支持非标准 G800/G801 把非模态命令映射为 `NonModal::KinematicsCoreXY` / `KinematicsCartesian`，并在执行时调用 `kinematics_set_mode()`。

注意:
- 这些 G-code 为自定义扩展，不属于标准 NIST G-code 集。
- 在使用前请确认 `Custom/CoreXY.cpp` 存在并正确编译到固件里。
