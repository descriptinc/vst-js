const { AudioContext } = require('web-audio-api')
// const Speaker = require('speaker')
const fs = require('fs')
const path = require('path')
const vstjs = require('./index')

// console.log('!!! vstjs', vstjs);

const bufferSize = 512
const numChannels = 2
const pluginPath = process.argv[2]
const filePath = process.argv[3]

if (!pluginPath) {
  throw new Error('no plugin path');
}
if (!filePath) {
  throw new Error('no file path');
}

// setup webaudio stuff
// const audioContext = new AudioContext()
// const sourceNode = audioContext.createBufferSource()
// const scriptNode = audioContext.createScriptProcessor(bufferSize, numChannels, numChannels)

// audioContext.outStream = new Speaker({
//   channels: audioContext.format.numberOfChannels,
//   bitDepth: audioContext.format.bitDepth,
//   sampleRate: audioContext.sampleRate,
// })

// const outFile = fs.createWriteStream('out.pcm');

// audioContext.outStream = outFile;

const sinWave = new Float32Array(bufferSize);
for (let i = 0; i < sinWave.length; i++) {
  sinWave[i] = Math.sin(1643.84 * Math.PI * (i / 44100));
}

const pluginHost = vstjs.launchPlugin(pluginPath)

pluginHost.processAudioBlock(1, bufferSize, [sinWave]);

console.log('!!! after');

// sourceNode.connect(scriptNode)
// scriptNode.connect(audioContext.destination)

// const pluginHost = vstjs.launchPlugin(pluginPath)
// // console.log('!!! plugin host', pluginHost);
// pluginHost.start()
// scriptNode.onaudioprocess = function onaudioprocess(audioProcessingEvent) {
//   const inputBuffer = audioProcessingEvent.inputBuffer
//   const channels = [...Array(numChannels).keys()]
//     .map(i => audioProcessingEvent.inputBuffer.getChannelData(i))

//   console.log('processing...', channels)

//   // process audio block via pluginHost
//   pluginHost.processAudioBlock(numChannels, bufferSize, channels)
//   audioProcessingEvent.outputBuffer = inputBuffer
// }



// fs.readFile(filePath, (err, fileBuf) => {
// //   console.log('reading file..')
//   if (err) throw err
//   audioContext.decodeAudioData(fileBuf, (audioBuffer) => {
//     sourceNode.buffer = audioBuffer
//     sourceNode.start(0)
//   }, (e) => { throw e })
// })