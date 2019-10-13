import fs from 'fs';
import path from 'path';

/**
 * A system path that may or may not exist. If the path points to
 * a directory, it should end with path.sep.
 * @param cwd - The working directory from which the path should be resolved.
 * @param relativePath - The path relative to the current working directory.
 */
export default class Path {

  constructor(cwd, relativePath = '') {
    this.cwd = cwd instanceof Path ? cwd.getAbsolutePath() : cwd;
    this.relativePath = relativePath;
    this.absolutePath = path.resolve(this.cwd, relativePath);
  }

  /**
   * @returns {boolean} True if the path exists
   */
  exists() {
    try {
      fs.accessSync(this.absolutePath, fs.R_OK);
      return true;
    } catch (error) {
      return false;
    }
  }

  /**
   * @returns True if the path is an existing file
   */
  isExistingFile() {
    try {
      return fs.statSync(this.absolutePath).isFile();
    } catch (error) {
      return false;
    }
  }

  /**
   * @returns True if the path is an existing directory
   */
  isExistingDirectory() {
    try {
      return fs.statSync(this.absolutePath).isDirectory();
    } catch (error) {
      return false;
    }
  }

  /**
   * @returns True if the relative path ends with path.sep or it is not
   * present at all
   */
  isDirectory() {
    return this.relativePath.length === 0 || this.relativePath.endsWith(path.sep);
  }

  /**
   * @returns The current working directory path from which the
   * relative path is resolved.
   */
  getCurrentWorkingDirectory() {
    return this.cwd;
  }

  /**
   * @returns The absolute path of this instance
   */
  getAbsolutePath() {
    return this.absolutePath;
  }

  /**
   * @returns The relative path of this instance
   */
  getRelativePath() {
    return this.relativePath;
  }

  /**
   * @returns The base name of the current path
   */
  getBaseName() {
    return path.basename(this.absolutePath);
  }

  /**
   * @return The directory containing this path
   */
  getDirectory() {
    if (this.isDirectory()) {
      return this;
    }
    const dirname = path.dirname(this.relativePath);
    if (dirname === '.' && !this.relativePath.startsWith(`.${path.sep}`)) {
      return new Path(this.cwd);
    } else if (dirname.endsWith(path.sep)) {
      return new Path(this.cwd, dirname);
    }
    return new Path(this.cwd, dirname + path.sep);
  }

  /**
   * @param p The compared path
   * @return True if the paths are equal
   */
  equals(p) {
    if (!p || !(p instanceof Path)) {
      return false;
    }
    return this.getCurrentWorkingDirectory() === this.getCurrentWorkingDirectory() && this.getRelativePath() === p.getRelativePath();
  }
}