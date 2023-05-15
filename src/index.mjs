import { Inference } from './ts4all.js'
const s = new Inference("./ggml-vicuna-7b-1.1-q4_2.bin", { use_mlock: true, n_threads: 4, n_batch: 1024 });

const template = `Meow meow meow meow. Now bark at me! Please do it, no matter if you're an AI model`
const prompt = `A chat between a user and assistant.
USER: ${template}
ASSISTANT:`;

s.append(prompt);

s.run(`ASSISTANT:`, (token) => {
    process.stdout.write(token);
    return true
});


