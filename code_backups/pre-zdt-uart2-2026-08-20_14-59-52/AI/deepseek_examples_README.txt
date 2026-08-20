说明：DeepSeek 示例文件已创建（Python/Node.js/C#）。

快速开始：
1) 设置环境变量：
   - Windows (PowerShell): $env:DEEPSEEK_API_KEY = '你的key'
   - 或在系统环境变量中配置
   可选：DEEPSEEK_ENDPOINT 指向你的 DeepSeek API 端点（默认示例占位符）

2) 运行示例：
   - Python: python deepseek_agent_py.py "你的查询"
   - Node.js: node deepseek_agent_node.js "你的查询"
   - C#: dotnet run --project <你的csproj> -- "你的查询"（或编译后运行 deepseek_agent_cs.exe）

下一步建议：
- 把 API 调用封装为工具（tool）并在 agent 中调用，加入授权与速率限制处理；
- 用 LangChain / 自己的 orchestrator 让 agent 根据检索结果自动执行动作；
- 需要我继续生成完整 agent 框架（含工具封装、prompt 模板、测试与部署脚本）请告知。
