// DeepSeek 最小接入示例（C#）
// 运行: dotnet run -- <query>
// 环境变量: DEEPSEEK_API_KEY, 可选 DEEPSEEK_ENDPOINT

using System;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;

class DeepSeekAgent {
    static async Task Main(string[] args) {
        var apiKey = Environment.GetEnvironmentVariable("DEEPSEEK_API_KEY");
        var endpoint = Environment.GetEnvironmentVariable("DEEPSEEK_ENDPOINT") ?? "https://api.deepseek.example/v1/search";
        if (string.IsNullOrEmpty(apiKey)) { Console.Error.WriteLine("请先设置环境变量 DEEPSEEK_API_KEY"); return; }
        var query = args.Length > 0 ? string.Join(" ", args) : "示例查询";

        using var client = new HttpClient();
        client.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", apiKey);
        var payload = $"{{\"query\":\"{query}\",\"top_k\":3}}";
        var content = new StringContent(payload, Encoding.UTF8, "application/json");
        var res = await client.PostAsync(endpoint, content);
        res.EnsureSuccessStatusCode();
        var body = await res.Content.ReadAsStringAsync();
        Console.WriteLine(body);
    }
}
