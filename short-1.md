---
layout: short
date: 2026-01-02
---
Looks like the first thing one needs when using `{% raw %}{{{% endraw %} page.content }}` is to get
rid of the enclosing `<p>...</p>`, while also keeping any nested paragraphs. So
`remove_first` but there's no `remove_last`. Reversing and removing the
reversed enclosing tag somehow works but does not return a string when `join`ing. So there's a local
string filter `chop_paragraph` ([\_plugins/custom\_filters.rb in git](_plugins/custom_filters.rb)).
