// DeepSeek 最小接入示例（Node.js）
// 需要 node 18+ 或安装 node-fetch: npm install node-fetch
// 环境变量: DEEPSEEK_API_KEY, 可选 DEEPSEEK_ENDPOINT

const fetch = require('node-fetch');
const API_KEY = process.env.DEEPSEEK_API_KEY;
const ENDPOINT = process.env.DEEPSEEK_ENDPOINT || 'https://api.deepseek.example/v1/search';

if (!API_KEY) {
  console.error('请先设置环境变量 DEEPSEEK_API_KEY');
  process.exit(1);
}

async function search(query, top_k = 3) {
  const res = await fetch(ENDPOINT, {
    method: 'POST',
    headers: {
      'Authorization': `Bearer ${API_KEY}`,
      'Content-Type': 'application/json'
    },
    body: JSON.stringify({ query, top_k })
  });
  if (!res.ok) throw new Error(await res.text());
  return res.json();
}

(async () => {
  const q = process.argv.slice(2).join(' ') || '示例查询';
  const r = await search(q);
  console.log(JSON.stringify(r, null, 2));
})();
