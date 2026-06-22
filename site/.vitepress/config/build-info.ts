import { execFileSync } from 'node:child_process'

// VitePress config 在 Node 运行，CI 已 fetch-depth: 0，可零依赖拿到 git 信号。
// 版本展示的唯一真相源是 git tag（package.json version 是停滞占位值，不可用）。
//
// date 用 git 提交日期（git log -1 --format=%ci HEAD 取首 10 字符 YYYY-MM-DD），稳定可复现；
// 不用运行时取当前时间的函数——构建脚本环境里不稳定也不可复现。

function git(args: string[]): string {
  try {
    return execFileSync('git', args, { encoding: 'utf8', stdio: ['ignore', 'pipe', 'ignore'] }).trim()
  } catch {
    return '' // 非 git 仓库 / 无 tag → 回退
  }
}

export interface BuildInfo {
  /** git describe 结果，如 v0.5.3 或 v0.5.3-3-gabc1234（-dirty 表示有未提交改动）；无 tag 退到 SHA；非 git 回退 dev */
  version: string
  /** 7 位短 SHA；非 git 仓库为空字符串 */
  sha: string
  /** 最近一次提交日期 YYYY-MM-DD（取自 git %ci）；非 git 回退 dev */
  date: string
}

export function getBuildInfo(): BuildInfo {
  return {
    version: git(['describe', '--tags', '--always', '--dirty']) || 'dev',
    sha: git(['rev-parse', '--short=7', 'HEAD']),
    date: git(['log', '-1', '--format=%ci', 'HEAD']).substring(0, 10) || 'dev',
  }
}
