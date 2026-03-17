#
# Qt 6.9.1 源码下载脚本 (Windows PowerShell)
# 用途: 下载Qt 6.9.1源码并解压到qt_src\qt6.9.1目录
#

# 错误时停止
$ErrorActionPreference = "Stop"

# 配置
$QtVersion = "6.9.1"
$QtMajor = "6.9"  # URL路径中的主版本号
$QtFile = "qt-everywhere-src-$QtVersion.zip"
$QtUrl = "https://mirrors.aliyun.com/qt/official_releases/qt/$QtMajor/$QtVersion/single/$QtFile"
# 备用官方源
$QtUrlOfficial = "https://download.qt.io/official_releases/qt/$QtMajor/$QtVersion/single/$QtFile"
$DestDir = "qt6.9.1"

# 获取脚本所在目录
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Set-Location $ScriptDir

Write-Host "========================================" -ForegroundColor Green
Write-Host "  Qt $QtVersion 源码下载脚本" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host ""

# 检查是否已存在
if (Test-Path $DestDir) {
    Write-Host "警告: $DestDir 目录已存在" -ForegroundColor Yellow
    $Response = Read-Host "是否删除并重新下载? (y/N)"
    if ($Response -eq 'y' -or $Response -eq 'Y') {
        Write-Host "删除现有目录..." -ForegroundColor Yellow
        Remove-Item -Recurse -Force $DestDir
    } else {
        Write-Host "取消操作" -ForegroundColor Yellow
        exit 0
    }
}

# 下载源码
Write-Host "[1/3] 下载 Qt $QtVersion 源码..." -ForegroundColor Green
Write-Host "URL: $QtUrl"

try {
    Invoke-WebRequest -Uri $QtUrl -OutFile $QtFile -UseBasicParsing -TimeoutSec 600
} catch {
    Write-Host "阿里云镜像下载失败，尝试官方源..." -ForegroundColor Yellow
    Invoke-WebRequest -Uri $QtUrlOfficial -OutFile $QtFile -UseBasicParsing -TimeoutSec 600
}

# 验证文件
Write-Host "[2/3] 验证下载文件..." -ForegroundColor Green
if (-not (Test-Path $QtFile)) {
    Write-Host "错误: 下载文件不存在" -ForegroundColor Red
    exit 1
}

$FileSize = (Get-Item $QtFile).Length / 1GB
Write-Host "文件大小: $([math]::Round($FileSize, 2)) GB"

# 解压
Write-Host "[3/3] 解压源码..." -ForegroundColor Green

# 使用 .NET Framework 解压（兼容性更好）
Add-Type -AssemblyName System.IO.Compression.FileSystem

# 创建临时解压目录
$TempExtractDir = "qt_extract_temp"
if (Test-Path $TempExtractDir) {
    Remove-Item -Recurse -Force $TempExtractDir
}

Write-Host "正在解压（可能需要几分钟）..."
[System.IO.Compression.ZipFile]::ExtractToDirectory(
    (Resolve-Path $QtFile).Path,
    (Join-Path $ScriptDir $TempExtractDir)
)

# 重命名目录
$ExtractedDir = Join-Path $ScriptDir "qt-everywhere-src-$QtVersion"
if (Test-Path $ExtractedDir) {
    Move-Item -Path $ExtractedDir -Destination $DestDir
} elseif (Test-Path (Join-Path $TempExtractDir "qt-everywhere-src-$QtVersion")) {
    Move-Item -Path (Join-Path $TempExtractDir "qt-everywhere-src-$QtVersion") -Destination $DestDir
}

# 清理临时目录
if (Test-Path $TempExtractDir) {
    Remove-Item -Recurse -Force $TempExtractDir
}

# 清理压缩包
$Response = Read-Host "是否删除压缩包 $QtFile? (y/N)"
if ($Response -eq 'y' -or $Response -eq 'Y') {
    Remove-Item $QtFile
    Write-Host "已删除压缩包" -ForegroundColor Green
}

# 显示结果
Write-Host ""
Write-Host "========================================" -ForegroundColor Green
Write-Host "  下载完成!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor Green
Write-Host "目录: $ScriptDir\$DestDir"
Write-Host ""

# 显示模块列表
if (Test-Path $DestDir) {
    Write-Host "包含的模块 (前15个):" -ForegroundColor Green
    $Modules = Get-ChildItem -Directory $DestDir | Select-Object -First 15 Name
    foreach ($Module in $Modules) {
        Write-Host "  $($Module.Name)"
    }
    $Total = (Get-ChildItem -Directory $DestDir).Count
    if ($Total -gt 15) {
        Write-Host "  ... (共 $Total 个模块/目录)"
    }
}
