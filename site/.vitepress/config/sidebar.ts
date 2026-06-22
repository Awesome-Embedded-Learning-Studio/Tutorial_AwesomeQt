import type { DefaultTheme } from 'vitepress'
import { readdirSync, statSync, readFileSync, existsSync } from 'fs'
import { join } from 'path'

type SidebarItem = DefaultTheme.SidebarItem

const DOCS_ROOT = join(import.meta.dirname, '../../../tutorial')

function extractTitle(filePath: string): string | null {
  try {
    const content = readFileSync(filePath, 'utf-8')
    const fmMatch = content.match(/^---[\s\S]*?^title:\s*['"]?(.+?)['"]?\s*$/m)
    if (fmMatch) return fmMatch[1]
    const h1 = content.match(/^#\s+(.+)$/m)
    if (h1) return h1[1].replace(/\{.*?\}/g, '').trim()
  } catch { /* ignore */ }
  return null
}

function humanize(name: string): string {
  return name
    .replace(/^\d+[-]?/, '')
    .replace(/[-_]/g, ' ')
    .replace(/\b\w/g, c => c.toUpperCase())
}

function sortEntries(a: string, b: string): number {
  const na = a.match(/^(\d+)/)?.[1]
  const nb = b.match(/^(\d+)/)?.[1]
  if (na && nb) return parseInt(na) - parseInt(nb)
  if (na) return -1
  if (nb) return 1
  return a.localeCompare(b, 'zh-CN')
}

function scanDir(dir: string, urlPrefix: string, depth = 0): SidebarItem[] {
  if (depth > 5) return []

  let entries: string[]
  try {
    entries = readdirSync(dir).filter(e =>
      !e.startsWith('.') &&
      e !== 'stylesheets' &&
      e !== 'hooks' &&
      e !== 'javascripts' &&
      e !== 'images' &&
      e !== 'public'
    )
  } catch { return [] }

  entries.sort(sortEntries)
  const items: SidebarItem[] = []

  for (const name of entries) {
    const fullPath = join(dir, name)
    if (!statSync(fullPath).isDirectory() && !name.endsWith('.md')) continue

    if (statSync(fullPath).isDirectory()) {
      const subItems = scanDir(fullPath, `${urlPrefix}/${name}`, depth + 1)
      const indexPath = join(fullPath, 'index.md')
      const title = extractTitle(indexPath) || humanize(name)

      if (subItems.length > 0) {
        items.push({
          text: title,
          link: existsSync(indexPath) ? `${urlPrefix}/${name}/` : undefined,
          items: subItems,
          collapsed: depth > 0,
        })
      } else if (existsSync(indexPath)) {
        items.push({ text: title, link: `${urlPrefix}/${name}/` })
      }
    } else if (name !== 'index.md' && name !== 'tags.md') {
      const title = extractTitle(fullPath) || humanize(name.replace(/\.md$/, ''))
      items.push({ text: title, link: `${urlPrefix}/${name.replace(/\.md$/, '')}` })
    }
  }

  return items
}

function hasContent(dir: string): boolean {
  try {
    return readdirSync(dir).some(e => e.endsWith('.md'))
  } catch { return false }
}

export function buildSidebar(): DefaultTheme.Sidebar {
  const sidebar: DefaultTheme.Sidebar = {}

  const beginnerDir = join(DOCS_ROOT, 'beginner')
  if (hasContent(beginnerDir)) {
    sidebar['/beginner/'] = scanDir(beginnerDir, '/beginner')
  }

  const engineeringDir = join(DOCS_ROOT, 'engineering')
  if (hasContent(engineeringDir)) {
    sidebar['/engineering/'] = scanDir(engineeringDir, '/engineering')
  }

  const advancedDir = join(DOCS_ROOT, 'advanced')
  if (hasContent(advancedDir)) {
    const items = scanDir(advancedDir, '/advanced')
    if (items.length > 0) {
      sidebar['/advanced/'] = items
    }
  }

  const expertDir = join(DOCS_ROOT, 'expert')
  if (hasContent(expertDir)) {
    const items = scanDir(expertDir, '/expert')
    if (items.length > 0) {
      sidebar['/expert/'] = items
    }
  }

  return sidebar
}
