export class APlaySdk {
  static version(): string {
    return '0.1.0';
  }

  static defaultReceiverName(): string {
    return 'APlay';
  }

  static createRuntime(): never {
    throw new Error('APlay Harmony runtime is TODO');
  }
}

export default APlaySdk;
