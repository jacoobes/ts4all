declare module "ts4all";

interface Params {
   seed: number; // RNG seed
   n_threads: number //= 0;
   n_ctx: number //= 2012; // Context size
   n_ctx_window_top_bar: number // 0; // Top bar of context window. Must be smaller than context size
   n_batch: number // 8; // Batch size
   n_repeat_last: number //0; // llama.cpp specific

   scroll_keep: number // 0.0f; // 0.4f to keep 40% of context below top bar when scrolling; 0.0f to remove everything after top bar
   top_k: number // 40;
   top_p: number// 0.9f;
   temp: number //  = 0.72f;
   repeat_penalty : number //= 1.0f; // llama.cpp specific
   eos_ignores : number //= 0; // llama.cpp specific
   use_mlock: boolean// true; // llama.cpp specific

}

declare class Inference {

  constructor(path: string, params?: Partial<Params>)
  append(info: string): void
  run(end: string, cb: (s: string) => boolean) : string
  create_savestate(): void //todo
  restore_savestate(): void //todo
  serialize(str: string): void
  deserialize() : void //todo
  get_prompt(): unknown //todo
  get_context_size() : unknown //todo
}


export { Inference, Params }
