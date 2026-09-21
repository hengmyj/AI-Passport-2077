const fs=require('fs'),vm=require('vm'),assert=require('assert');
const source=fs.readFileSync(require('path').join(__dirname,'../configure/profile-core.js'),'utf8');
let delays=0;const scope={TextEncoder,TextDecoder,Uint8Array,Uint8ClampedArray,setTimeout:fn=>{delays++;fn()}};
vm.createContext(scope);vm.runInContext(source+'\nthis.helpers={deviceError,beginProfileUpload};',scope);
(async()=>{
 const {deviceError,beginProfileUpload}=scope.helpers;
 const args={badge:1,baseRevision:42,profile:{qrPresent:true},format:4};let calls=0;
 const transient={rpc:async(op,p)=>{assert.equal(op,'begin');assert.strictEqual(p,args);if(++calls<4)throw deviceError({error:'ESP_ERR_INVALID_STATE',reason:'display_pending'});return {ok:true}}};
 assert((await beginProfileUpload(transient,args)).ok);assert.equal(calls,4);assert.equal(delays,3);
 for(const reason of ['profile_changed','upload_busy',undefined]){
  calls=0;await assert.rejects(()=>beginProfileUpload({rpc:async()=>{calls++;throw deviceError({error:'ESP_ERR_INVALID_STATE',reason})}},args));assert.equal(calls,1);assert.equal(args.baseRevision,42);
 }
 calls=0;await assert.rejects(()=>beginProfileUpload({rpc:async()=>{calls++;throw deviceError({error:'ESP_ERR_INVALID_STATE',reason:'display_pending'})}},args));assert.equal(calls,16);
 assert(deviceError({reason:'profile_changed'}).message.includes('当前编辑已保留'));
 console.log('Upload recovery PASS: bounded display retry, unchanged revision, no retry for conflicts/other sessions/unknown failures');
})().catch(e=>{console.error(e);process.exitCode=1});
