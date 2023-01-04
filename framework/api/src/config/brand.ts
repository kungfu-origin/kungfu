import path from 'path';
import fse from 'fs-extra';

export const getLogoPath = () => {
  const replaceLogoPath = path.join(
    globalThis.__publicResources,
    'logo',
    'logo-replace.png',
  );

  if (!fse.existsSync(replaceLogoPath)) {
    return null;
  } else {
    return replaceLogoPath;
  }
};

export const getDialogLogoPath = () => {
  const replaceLogoPath = path.join(
    globalThis.__publicResources,
    'logo',
    'dialog-logo-replace.png',
  );
  const defaultLogoPath = path.join(
    globalThis.__publicResources,
    'logo',
    'dialog-logo.png',
  );

  if (!fse.existsSync(replaceLogoPath)) {
    return defaultLogoPath;
  } else {
    return replaceLogoPath;
  }
};

export const isDefaultLogo = () => {
  const replaceLogoPath = path.join(
    globalThis.__publicResources,
    'logo',
    'logo-replace.png',
  );
  if (!fse.existsSync(replaceLogoPath)) {
    return true;
  } else {
    return false;
  }
};
