// @flow

import fs from 'fs';
import path from 'path';
import Path from './Path';

/**
 * State machine for a path autocomplete UI.
 * @param {string} cwd - The working directory from which path should be resolved
 * @param {boolean} [directoryOnly=false] - If set to should handle only directories
 */
export default class PathAutocomplete {
  cwd: Path;
  directoryOnly: boolean;
  path: Path;
  matches: ?Path[];
  matchIndex: number;

  constructor(cwd: string, directoryOnly: boolean = false) {
    this.cwd = new Path(cwd);
    if (!this.cwd.isExistingDirectory()) {
      throw new Error(`The provided working directory ${cwd} does not exist or is not a directory.`);
    }
    this.path = this.cwd;
    this.matches = null;
    this.matchIndex = -1;
    this.directoryOnly = directoryOnly;
  }

  /**
   * Reset the state of the class
   */
  reset(): void {
    this.path = this.cwd;
    this.matches = null;
    this.matchIndex = -1;
  }

  /**
   * @return The current input path
   */
  getPath(): Path {
    return this.path;
  }

  /**
   * @return The currently selected path or the path
   */
  getActivePath(): Path {
    if (this.matches == null) {
      return this.path;
    }
    const match = this.matches[this.matchIndex];
    if (match == null) {
      return this.path;
    }
    return match;
  }

  /**
   * @return True if the autocomplete is for directory only
   */
  isDirectoryOnly(): boolean {
    return this.directoryOnly;
  }

  /**
   * @return The current matches
   */
  getMatches(): ?Path[] {
    return this.matches;
  }

  /**
   * @returns The current match index or -1 if no match is selected;
   */
  getMatchIndex(): number {
    return this.matchIndex;
  }

  /**
   * @returns The current working directory
   */
  getWorkingDirectory(): Path {
    return this.cwd;
  }

  /**
   * Set the path based on the user input
   * @param input
   */
  setPath(input: string): void {
    const inputPath = new Path(this.cwd, input);
    if (this.path.equals(inputPath)) {
      return;
    }
    this.path = inputPath;
    this.matches = null;
    this.matchIndex = -1;
  }

  /**
   * Reset the matches/match index
   */
  cancelMatch(): void {
    this.matches = null;
    this.matchIndex = -1;
  }

  /**
   * Use the current match index (or the provided one) to update the
   * current path
   * @param [matchIndex=this.matchSelectionIndex]
   */
  selectMatch(matchIndex: ?number): void {
    if (!this.matches) {
      throw new Error('There are not potential matches');
    }
    const index = matchIndex == null ? this.matchIndex : matchIndex;
    if (index == null) {
      throw new Error('The match index is null');
    }
    const match = this.matches[index];
    if (match == null) {
      throw new Error('The match index does not exist');
    }
    this.path = match;
    this.matches = null;
    this.matchIndex = -1;
  }

  /**
   * Go to the next potential path. If there is only one match, the current path is set to
   * that match.
   * @param forward - If true, move to the next match. Otherwise move to the previous match.
   */
  nextMatch(forward: boolean = true): void {
    const matchesExist = !!this.matches;
    this.refreshMatches();
    if (!this.matches) {
      return;
    }
    if (this.matches.length === 1) {
      this.selectMatch(0);
      return;
    }
    // If the matches were inactive, do not update the selection yet
    if (!matchesExist) {
      return;
    }
    // Update the selection index while making sure that it is
    // a valid index within the matches length
    const position = this.matchIndex + ((forward) ? 1 : -1);
    const boundedPosition = position % this.matches.length;
    this.matchIndex = (position >= 0) ? boundedPosition : this.matches.length + boundedPosition;
  }

  /**
   * Refresh the matches for the current path. NOOP if the matches have already been
   * computed
   */
  refreshMatches() {
    if (this.matches !== null) {
      return;
    }
    this.matches = this.findMatches();
  }

  /**
   * Find a list of existing file system files or directory matching the current
   * input path. If multiple files/directories share the same prefix, a single entry
   * with the shared prefix is returned.
   */
  findMatches(): ?Path[] {
    const directory = this.path.getDirectory();
    if (!directory.isExistingDirectory()) {
      return null;
    }
    // For a directory, no need to filter on a prefix.
    const prefix = this.path.isDirectory() ? '' : this.path.getBaseName();
    const matches = fs.readdirSync(directory.getAbsolutePath())
      // Filter on the base input name
      .filter(child => child.startsWith(prefix))
      // Map to Path instances
      .map((name) => {
        const childPath = directory.getRelativePath() + name;
        try {
          if (fs.statSync(childPath).isDirectory()) {
            return new Path(this.cwd, childPath + path.sep);
          }
        } catch (ex) {
          // Be more selective
        }
        return new Path(this.cwd, childPath);
      })
      // Get rid of non directory if the ShellPath handles directories only
      .filter((child: Path) => !this.directoryOnly || child.isExistingDirectory())
      // Sort by directory/file and then name
      .sort((child1: Path, child2: Path) => {
        if (child1.isDirectory() !== child2.isDirectory()) {
          return child1.isExistingDirectory() ? 1 : -1;
        }
        return child1.getAbsolutePath().localeCompare(child2.getAbsolutePath());
      });
    if (matches.length === 0) {
      return null;
    }
    if (matches.length === 1) {
      return matches;
    }
    const commonMatch = PathAutocomplete.findCommonMatch(matches);
    if (commonMatch == null) {
      return matches;
    }
    return !commonMatch.equals(this.path) ? [commonMatch] : matches;
  }

  /**
   * Find a common prefix for the provided paths
   * @private
   * @returns {Path} A common path to all the potential paths
   */
  static findCommonMatch(matches: Path[]): ?Path {
    // Sort the paths. The 2 extremes are the most different.
    const sortedName = matches
      .map(p => p.getRelativePath())
      .sort();
    const a1 = sortedName[0];
    const a2 = sortedName[sortedName.length - 1];
    // Find the common prefix between a1 and a2.
    let i = 0;
    while (i < a1.length && a1.charAt(i) === a2.charAt(i)) {
      i += 1;
    }
    const prefix = a1.substr(0, i);
    if (i === 0) {
      return null;
    }
    return new Path(matches[0].getCurrentWorkingDirectory(), prefix);
  }
}
