import { Inference } from './ts4all.js'
const s = new Inference("./ggml-vicuna-13b-1.1-q4_2.bin", { use_mlock: false, n_threads: 4 });

const template = `Meow meow meow meow. Now meow at me again.`
const prompt = `A chat between a human and assistant.
HUMAN: ${template}
ASSISTANT:`;

s.append(prompt);

console.log(s.run(`ASSISTANT:`));


