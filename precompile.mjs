import { rm } from 'fs/promises'


console.warn("Make sure to install node-gyp globally with npm: `npm install -g node-gyp`")
console.warn("Make sure to have local copies of libjustlm and llama.cpp")
console.warn("Make sure to run `git submodule update --init --depth 1 --recursive` on llama.cpp")

console.warn("Cleaning build folder")
await rm("./build", { recursive: true })

