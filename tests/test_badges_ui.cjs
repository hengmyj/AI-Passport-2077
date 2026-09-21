const fs=require('fs'),vm=require('vm'),assert=require('assert');
const source=fs.readFileSync(require('path').join(__dirname,'../configure/app.js'),'utf8');
class Element{constructor(){this.children=[];this.dataset={};this.value='';this.elements=[];}append(...x){this.children.push(...x)}replaceChildren(){this.children=[]}querySelectorAll(){return this.children}}
const elements={},get=id=>elements[id]??=new Element(),calls=[];
const profile={name:'Example',theme:{}};
const catalog={badgeCount:5,activeBadge:0,badges:Array.from({length:5},(_,id)=>({id,name:id===2?'<img src=x>':'Card',configured:id<3,revision:4}))};
const ctx=(w,h)=>({getImageData:()=>({width:w,height:h})});
const scope={yaoSupported:false,yaoDirty:false,maxProfileVersion:4,maxBadgeLayout:2,fontsReady:Promise.resolve(),beginProfileUpload:(d,a)=>d.rpc('begin',a),portraitPayload:()=>new Uint8Array(91648),document:{createElement:()=>new Element(),fonts:{ready:Promise.resolve()}},$:get,editingBadge:2,activeBadge:0,badgeCount:5,badges:[],loadedBadgeRevision:4,staleBadge:false,busy:false,scanBusy:false,dirty:false,wifi:{},viaHotspot:false,device:{closed:false,rpc:async(op,args)=>{calls.push([op,args]);return op==='info'?{...catalog,custom:true,profile,badgeRevision:5}:catalog}},fields:()=>profile,validateExtended:p=>p,render:()=>{},message:()=>{},cc:ctx(208,148),av:ctx(72,88),bc:ctx(148,44),lc:ctx(32,32),qc:ctx(192,192),qrReady:false,to565:x=>new Uint8Array(x.width*x.height*2),Uint8Array,FORMAT:{chunk:768},btoa:s=>Buffer.from(s,'binary').toString('base64'),setTimeout:f=>f()};
vm.createContext(scope);
vm.runInContext(source.slice(source.indexOf('function controls()'),source.indexOf('function drawLogo('))+source.slice(source.indexOf('function showCatalog('),source.indexOf('async function load(')),scope);
scope.catalog=catalog;vm.runInContext('showCatalog(catalog)',scope);
assert(get('badgeSelect').children[2].textContent.includes('<img src=x>'));assert(!get('displayBadge').disabled);assert.equal(scope.editingBadge,2);
catalog.activeBadge=1;vm.runInContext('showCatalog(catalog)',scope);assert.equal(scope.editingBadge,2);assert(!scope.staleBadge);
catalog.badges[2].revision=5;vm.runInContext('showCatalog(catalog)',scope);assert(scope.staleBadge);assert(get('save').disabled);
catalog.badges[2].revision=4;vm.runInContext('showCatalog(catalog)',scope);
vm.runInContext(source.slice(source.indexOf("$('save').onclick="),source.indexOf("for(const id of ['ssid','wifiPassword'")),scope);
(async()=>{scope.maxBadgeLayout=1;await get('save').onclick();assert(!calls.some(c=>c[0]==='begin'));scope.maxBadgeLayout=2;calls.length=0;await get('save').onclick();const begin=calls.find(c=>c[0]==='begin');assert.equal(begin[1].badge,2);assert.equal(begin[1].format,4);assert.equal(begin[1].baseRevision,4);assert.equal(calls.find(c=>c[0]==='info')[1].badge,2);assert(!calls.some(c=>c[0]==='badge_select'));assert(calls.some(c=>c[0]==='commit'));console.log('Multi-badge UI: PASS (independent editing, explicit write target, stale-write guard, safe names)')})().catch(e=>{console.error(e);process.exitCode=1});
