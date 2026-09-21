const fs=require('fs'),vm=require('vm'),assert=require('assert');
const source=fs.readFileSync(require('path').join(__dirname,'../configure/app.js'),'utf8');
class Element{constructor(){this.children=[];this.dataset={};this.value='';this.elements=[];this.style={};}append(...items){this.children.push(...items)}replaceChildren(){this.children=[]}querySelectorAll(){return this.children}}
const elements={};const get=id=>elements[id]??=new Element();
const scope={yaoSupported:false,yaoDirty:false,document:{createElement:()=>new Element()},$:get,networks:[{ssid:'Office',rssi:-42,security:'WPA2',supported:true,open:false},{ssid:'Guest',rssi:-65,security:'OPEN',supported:true,open:true},{ssid:'Enterprise',rssi:-60,security:'Enterprise',supported:false,open:false},{ssid:'<img src=x onerror=bad()>',rssi:-70,security:'WPA2',supported:true,open:false}],wifi:{ssid:'Office',passwordSet:true},wifiDirty:false,busy:false,scanBusy:false,dirty:false,device:{closed:false},viaHotspot:false,badgeCount:1,badges:[],editingBadge:0,activeBadge:0,staleBadge:false};
vm.createContext(scope);
vm.runInContext(source.slice(source.indexOf('function controls()'),source.indexOf('function drawLogo('))+source.slice(source.indexOf('function renderNetworks()'),source.indexOf('async function scanWifi()'))+'\nrenderNetworks();',scope);
get('ssid').value='Office';get('wifiPassword').value='draft-password';get('networkList').children[0].onclick();assert.equal(get('wifiPassword').value,'draft-password');
get('networkList').children[1].onclick();assert.equal(get('ssid').value,'Guest');assert.equal(get('wifiPassword').value,'');assert.equal(get('openWifi').checked,true);assert.equal(scope.wifiDirty,true);
assert.equal(get('networkList').children[2].disabled,true);assert.equal(get('networkList').children[3].children[0].textContent,'<img src=x onerror=bad()>');
scope.device.closed=true;vm.runInContext('controls()',scope);assert(get('networkList').children.every(x=>x.disabled));assert(get('scanWifi').disabled);
console.log('Wi-Fi list UI: PASS (selection, password clearing, open network, unsupported/closed states, literal SSID text)');
