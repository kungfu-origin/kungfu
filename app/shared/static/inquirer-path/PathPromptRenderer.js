import chalk from 'chalk';
import path from 'path';
import readline from 'readline';
import ScreenManager from 'inquirer/lib/utils/screen-manager';

import PathAutocomplete from './PathAutocomplete';

const RANGE_SIZE = 5;

/**
 * Render the path prompt UI based on a instance of { @link PathAutocomplete }
 */
export default class PathPromptRenderer {

  constructor(rl, screen, autocomplete, message) {
    this.rl = rl;
    this.screen = screen;
    this.message = message;
    this.autocomplete = autocomplete;
  }

  /**
   * Restore the state of the resources used by the renderer
   */
  kill() {
    this.screen.done();
  }

  /**
   * Render the prompt UI
   * @param [finalAnswer=null] If present, display the final answer
   */
  render(finalAnswer) {
    const message = this.buildMainContent(finalAnswer);
    const bottom = finalAnswer ? '' : this.buildBottomContent();
    this.screen.render(message, bottom);
    if (!finalAnswer) {
      // Reset the line value to match the state of the PathAutocomplete instance
      this.resetCursor();
    }
  }

  /**
   * Render the UI for a new prompt. It finalizes the current render,
   * inserts a new line and render a new path prompt.
   * @param finalAnswer
   * @param autocomplete The new autocomplete state instance
   */
  renderNewPrompt(finalAnswer, autocomplete) {
    // Finalize the current render
    this.render(finalAnswer);
    // Add a new line to keep the rendered answer
    this.rl.output.unmute();
    this.rl.output.write('\n');
    this.rl.output.mute();
    this.autocomplete = autocomplete;
    // Render the new prompt
    this.render();
  }

  /**
   * Render the error UI
   * @param error
   */
  renderError(error) {
    this.screen.render(this.buildMainContent(), chalk.red('>> ') + error);
    this.resetCursor();
  }

  /**
   * Render the main content of the prompt. The message includes the question and
   * the current response.
   * @param [finalAnswer=null] If present, display the final answer
   */
  buildMainContent(finalAnswer) {
    let message = `${chalk.green('?')} ${chalk.bold(this.message)}${chalk.reset(' ')}`;
    message += chalk.dim(`(${this.autocomplete.getWorkingDirectory().getBaseName()}) `);
    if (finalAnswer) {
      message += chalk.cyan(finalAnswer);
    } else {
      message += this.autocomplete.getActivePath().getRelativePath();
    }
    return message;
  }

  /**
   * Render the bottom content of the prompt. It displays the current
   * selection state of the {@link PathAutocomplete} instance
   * @returns {string}
   */
  buildBottomContent() {
    const matches = this.autocomplete.getMatches();
    if (matches == null) {
      return '';
    }
    const matchIndex = this.autocomplete.getMatchIndex();
    const match = matches[matchIndex];

    return PathPromptRenderer.shortenArray(matches, matchIndex, RANGE_SIZE).map(potentialPath => {
      const suffix = potentialPath.isDirectory() ? path.sep : '';
      if (potentialPath === match) {
        return chalk.black.bgWhite(potentialPath.getBaseName() + suffix);
      }
      const colorize = potentialPath.isDirectory() ? chalk.red : chalk.green;
      return colorize(potentialPath.getBaseName()) + suffix;
    }).join('\n');
  }

  /**
   * Reset the input cursor to the end of the line
   */
  resetCursor() {
    // Move the display cursor
    const activeEntry = this.autocomplete.getActivePath().getRelativePath();
    if (activeEntry === this.rl.line) {
      return;
    }
    const cursorPosition = (this.autocomplete.getWorkingDirectory().getBaseName().length + this.message.length + activeEntry.length + 6) % this.rl.output.columns;

    this.rl.line = activeEntry;
    this.rl.output.unmute();
    readline.cursorTo(this.rl.output, cursorPosition);
    this.rl.cursor = activeEntry.length;
    this.rl.output.mute();
  }

  /**
   * Slice an array around a specific item so that it contains a specific number of elements.
   * @param items - The array to shorten
   * @param itemIndex - The index of the item that should be included in the returned slice
   * @param size - The desired size of the array to be returned
   * @returns
   */
  static shortenArray(items, itemIndex, size) {
    const length = items.length;
    let min = itemIndex - Math.floor(size / 2);
    let max = itemIndex + Math.ceil(size / 2);
    if (min < 0) {
      max = Math.min(length, max - min);
      min = 0;
    } else if (max >= length) {
      min = Math.max(0, min - (max - length));
      max = length;
    }
    return items.slice(min, max);
  }
}