"""DeepSeek 最小接入示例（Python）
依赖: pip install requests
设置环境变量: DEEPSEEK_API_KEY, 可选 DEEPSEEK_ENDPOINT
替换 ENDPOINT 为你的 deepSeek 实际端点。
"""
import os
import sys
import json
import requests

API_KEY = os.getenv("DEEPSEEK_API_KEY")
ENDPOINT = os.getenv("DEEPSEEK_ENDPOINT", "https://api.deepseek.example/v1/search")

if not API_KEY:
    print("请先设置环境变量 DEEPSEEK_API_KEY")
    sys.exit(1)


def search(query, top_k=3):
    headers = {"Authorization": f"Bearer {API_KEY}", "Content-Type": "application/json"}
    payload = {"query": query, "top_k": top_k}
    r = requests.post(ENDPOINT, headers=headers, json=payload)
    r.raise_for_status()
    return r.json()


if __name__ == '__main__':
    q = " ".join(sys.argv[1:]) or "示例查询"
    print(json.dumps(search(q), ensure_ascii=False, indent=2))
