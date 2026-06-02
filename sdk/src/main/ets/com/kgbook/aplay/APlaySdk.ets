import { defaultReceiverName, version } from 'libaplay_napi.so';

export class APlaySdk {
  static version(): string {
    return version();
  }

  static defaultReceiverName(): string {
    return defaultReceiverName();
  }

  static createRuntime(): never {
    throw new Error('APlay Harmony runtime is TODO');
  }
}

export default APlaySdk;
