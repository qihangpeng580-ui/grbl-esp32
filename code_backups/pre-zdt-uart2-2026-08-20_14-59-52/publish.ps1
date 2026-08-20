# publish.ps1 - Grbl_Esp32 版本发布脚本
# 用法: .\publish.ps1 -Version "v1.1" -Message "修复进纸电机丢步"
param(
    [Parameter(Mandatory=$true)] [string]$Version,
    [Parameter(Mandatory=$true)] [string]$Message
)

$date = Get-Date -Format "yyyy-MM-dd"
$tag = "${Version}_${date}"

Write-Host "=== Grbl_Esp32 版本发布 ===" -ForegroundColor Cyan
Write-Host "版本: $Version" 
Write-Host "日期: $date"
Write-Host "Tag:  $tag"
Write-Host "说明: $Message"
Write-Host ""

# 1. 添加所有变更
Write-Host "[1/4] git add -A" -ForegroundColor Yellow
git add -A

# 2. 提交
Write-Host "[2/4] git commit" -ForegroundColor Yellow
git commit -m "$tag`: $Message"

# 3. 打标签
Write-Host "[3/4] git tag" -ForegroundColor Yellow
git tag -a $tag -m "$Version`: $Message"

# 4. 推送
Write-Host "[4/4] git push" -ForegroundColor Yellow
git push
git push origin $tag

Write-Host ""
Write-Host "=== 发布完成 ===" -ForegroundColor Green
Write-Host "仓库: https://github.com/qihangpeng580-ui/grbl-esp32"
Write-Host "版本: $tag"
