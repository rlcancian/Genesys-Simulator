#!/usr/bin/env python3
"""Conservative static inventory. Candidate classifications require review."""
from pathlib import Path
import re, csv, json, collections
R=Path("source/plugins")
FR=re.compile(r'(?P<type>\b(?:std::)?string\b|\bQString\b|\bList\s*<\s*(?:std::)?string\s*>|\bstd::(?:vector|list|deque|set|unordered_set)\s*<\s*(?:std::)?string\s*>|\bstd::(?:map|unordered_map|multimap)\s*<\s*(?:std::)?string\s*,\s*(?:std::)?string\s*>)\s*[&*]?\s*(?P<name>[a-zA-Z_]\w*)\s*(?=\s*[;=\{])')
CR=re.compile(r'\b(?:bool|void)\s+(?P<cls>\w+)::(?P<method>_check|checkSymbols)\s*\([^)]*\)\s*(?:const\s*)?\{')
PR=re.compile(r'\b(?:checkExpression|parseExpression|validate\w*Expression|check\w*Expression)\s*\(')
SR=re.compile(r'new\s+SimulationControl(?:Generic\s*<\s*std::string\s*>|String)\s*\(')
def clean(s): return re.sub(r'//[^\n]*','',re.sub(r'/\*.*?\*/','',s,flags=re.S))
def body(s,at):
    level=1; i=at+1; start=i; quote=''; escaped=False
    while i<len(s) and level:
        c=s[i]
        if quote:
            if escaped: escaped=False
            elif c=='\\': escaped=True
            elif c==quote: quote=''
        elif c in ('"',"'"): quote=c
        elif c=='{':level+=1
        elif c=='}':level-=1
        i+=1
    return s[start:i-1]
def fields(h):
    out=[]
    for n,line in enumerate(clean(h).splitlines(),1):
        t=line.strip()
        if not t or t.startswith('#') or 'typedef ' in t or 'using ' in t:continue
        for m in FR.finditer(t):
            nm=m.group('name')
            if '(' in t[:m.start()] or re.search(r'\b'+re.escape(nm)+r'\s*\(',t):continue
            if nm.lower().lstrip('_') in ('name','label'):continue
            out.append((nm,re.sub(r'\s+','',m.group('type')),n))
    return list(dict.fromkeys(out))
def controls(s):
    out=[]
    for m in SR.finditer(s):
        seg=s[m.end():m.end()+750].split(';',1)[0]
        literals=re.findall(r'"([^"\\]*(?:\\.[^"\\]*)*)"',seg)
        if len(literals)>=2 and literals[-2] not in out:out.append(literals[-2])
    return out
def status(nm,s,b):
    expr=[l.strip() for l in b.splitlines() if PR.search(l)]
    direct=any(re.search(r'\b(?:checkExpression|parseExpression)\s*\(\s*(?:this\s*->\s*)?'+re.escape(nm)+r'\b',l) for l in expr)
    indirect=False
    if nm in b and any('checkExpression' in l for l in expr):
        indirect=bool(re.search(r'\bfor\s*\([^;]*'+re.escape(nm),b,flags=re.S))
        indirect |= bool(re.search(re.escape(nm)+r'\s*->\s*(?:begin|list)\s*\(',b))
        indirect |= bool(re.search(r'\bcheckExpression\s*\(\s*(?:get\w*\s*\(|\(\*\w+\)\.second)',b))
    helper=nm in b and any(re.search(r'\b(?:validate|check)\w*Expression\s*\(',l) and 'checkExpression(' not in l for l in expr)
    runtime=bool(re.search(r'\bparseExpression\s*\(\s*(?:this\s*->\s*)?'+re.escape(nm)+r'\b',s))
    return 'DIRECT_CHECK' if direct else 'INDIRECT_CANDIDATE' if indirect else 'HELPER_CANDIDATE' if helper else 'MENTIONED_NOT_PARSER_PROVEN' if nm in b else 'RUNTIME_PARSE_ONLY' if runtime else 'NOT_EVIDENCED'
def main():
    data=[]; overview=[]
    for p in sorted(R.rglob('*.cpp')):
        s=p.read_text(errors='replace'); h=p.with_suffix('.h'); hs=h.read_text(errors='replace') if h.exists() else ''
        cls=p.stem; stripped=clean(s)
        checks=[body(stripped,m.end()-1) for m in CR.finditer(stripped) if m.group('cls')==cls]
        cb='\n'.join(checks); fs=fields(hs); props=controls(stripped)
        calls=[l.strip() for l in cb.splitlines() if PR.search(l)][:16]
        info={'class':cls,'category':p.parts[2], 'path':str(p),'checks':bool(checks),
            'calls':calls,'controls':props,
            'fields':[[nm,status(nm,stripped,cb),ty,ln] for nm,ty,ln in fs]}
        overview.append(info)
        for nm,ty,ln in fs:
            data.append({'category':p.parts[2],'class':cls,'attribute':nm,'type':ty,'header_line':ln,
             'status':status(nm,stripped,cb),'name_has_Expression':'Expression' in nm,
             'check_defined':bool(checks),'gui_string_properties':'; '.join(props),
             'check_calls':' | '.join(calls),'file':str(p)})
    Path('plugin-expression-inventory.json').write_text(json.dumps(overview,ensure_ascii=False,indent=2))
    with Path('plugin-expression-inventory.csv').open('w',newline='') as fh:
        writer=csv.DictWriter(fh,fieldnames=['category','class','attribute','type','header_line','status','name_has_Expression','check_defined','gui_string_properties','check_calls','file'])
        writer.writeheader();writer.writerows(data)
    print('AUDIT_META',json.dumps({'cpp_files':len(overview),'fields':len(data),'with_check':sum(bool(x['checks']) for x in overview),
       'statuses':dict(collections.Counter(x['status'] for x in data))}))
    for item in overview:
        print('AUDIT_ROW',json.dumps(item,ensure_ascii=False,separators=(',',':')))
if __name__=='__main__':main()
