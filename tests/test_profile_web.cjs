const fs=require('fs'),vm=require('vm'),assert=require('assert');
const html=fs.readFileSync(require('path').join(__dirname,'../configure/index.html'),'utf8');
const firmwareLayout=fs.readFileSync(require('path').join(__dirname,'../main/badge_layout.h'),'utf8');
const deviceRegions=[...firmwareLayout.matchAll(/\{\s*(\d+(?:\s*,\s*\d+){6})\s*\}/g)].map(m=>m[1].split(',').map(Number));
const previewRegions=JSON.parse(html.match(/const badgeRegions=(\[[^;]+);/)[1]);
assert.deepStrictEqual(previewRegions,deviceRegions);
console.log('Preview regions match firmware crop, position and scale: PASS');
const scripts=[...html.matchAll(/<script(?: [^>]*)?>([\s\S]*?)<\/script>/g)].map(m=>m[1]);
for(const source of scripts)new vm.Script(source);
const scope={TextEncoder,TextDecoder,Uint8Array,Uint8ClampedArray,console};vm.createContext(scope);
vm.runInContext(scripts[0]+`;this.test={to565,rgba565,validateProfile,fit,nameLayout,drawName};`,scope);
const x=scope.test.to565({width:3,height:1,data:[255,0,0,255,0,255,0,255,0,0,255,255]});
assert.deepEqual([...x],[0,248,224,7,31,0]);assert.deepEqual([...scope.test.rgba565(x,3,1)],[255,0,0,255,0,255,0,255,0,0,255,255]);
assert.throws(()=>scope.test.rgba565(x,2,1));
const p={name:'测试员工',department:'研发部',title:'工程师',employeeId:'NC-01',signature:'保持好奇'};scope.test.validateProfile(p);assert.throws(()=>scope.test.validateProfile({...p,name:''}));assert.throws(()=>scope.test.validateProfile({...p,name:'a\n'}));assert.throws(()=>scope.test.validateProfile({...p,signature:'汉'.repeat(44)}));
console.log('Profile web checks: PASS (script syntax, RGB565 roundtrip, Unicode, validation)');

vm.runInContext(scripts[1]+'\n'+scripts[2],scope);
vm.runInContext('this.ext={qrPixels,qrDisplayPixels,packQr,unpackQr,validateExtended,DEFAULT_PROFILE};',scope);
const payload=Array.from(Buffer.from('https://example.com/test-contact'));
const pixels=scope.ext.qrPixels(payload);
const packed=scope.ext.packQr({width:192,height:192,data:pixels});
assert.equal(packed.length,4608);
const unpacked=scope.ext.unpackQr(packed);
assert.deepEqual([...pixels],[...unpacked]);
assert.deepEqual([...scope.jsQR(unpacked,192,192).binaryData],payload);
// Check actual decode after display resampling, including dense binary codes.
for(const n of [12,50,100,220,350,500]){
 const bytes=Array.from({length:n},(_,i)=>(i*73+41)%256);
 const view=scope.ext.qrDisplayPixels(scope.ext.qrPixels(bytes));
 assert.equal(view.length,176*176*4);
 assert.deepEqual([...scope.jsQR(view,176,176).binaryData],bytes);
}
assert.throws(()=>scope.ext.qrPixels(new Array(1300).fill(65)));
scope.ext.validateExtended(scope.ext.DEFAULT_PROFILE);
assert.throws(()=>scope.ext.validateExtended({...scope.ext.DEFAULT_PROFILE,brandName:''}));
assert.throws(()=>scope.ext.validateExtended({...scope.ext.DEFAULT_PROFILE,theme:{}}));
console.log('Extended web checks: PASS (QR encode/decode, 1-bit packing, capacity, theme validation)');

// Deterministic font metrics exercise whitespace, long tokens and CJK wrapping.
const metric={font:'',measureText(text){const size=Number(this.font.match(/(\d+)px/)[1]);return {width:Array.from(text).reduce((n,c)=>n+(c===' '?0.3:c.charCodeAt(0)>127?1:0.65)*size,0)}}};
let layout=scope.test.nameLayout(metric,'FOLO MAKER');assert.deepEqual(Array.from(layout.lines),['FOLO','MAKER']);assert(!layout.truncated);
for(const name of ['艾丽丝张晓明测试','ABCDEFGHIJKLMNOPQRST','ZOE','FOLO   MAKER','𠮷野测试姓名']){
 const result=scope.test.nameLayout(metric,name);assert(result.lines.length<=2);assert(!result.truncated);
 assert(result.lines.join('').replace(/ /g,'')===name.replace(/ /g,''));
 assert(result.lines.every(s=>metric.measureText(s).width<=112));
}
layout=scope.test.nameLayout(metric,'A'.repeat(120));assert(layout.truncated);assert(layout.lines.length===2);assert(layout.lines[1].endsWith('…'));
console.log('Name wrapping: PASS (word breaks, CJK, long words, Unicode, bounded two-row layout)');
