// @flow
import readline from 'readline';
import {
  Writable as WritableStream,
} from 'stream';

declare class MutableStream extends WritableStream {
  mute: () => void,
  unmute: () => void,
}
declare class ReadLineInterface extends readline.Interface {
  line: string,
  cursor: number,
  output: MutableStream
}