module.exports = {
  root: true,
  parser: '@typescript-eslint/parser',
  env: {
    node: true,
  },
  extends: ['eslint:recommended'],
  plugins: ['@typescript-eslint'],
  parserOptions: {
    ecmaVersion: 2020,
  },
  rules: {
    'no-console': 'off',
    'no-undef': 'off',
    'no-control-regex': 0,
    '@typescript-eslint/no-var-requires': 'off',
  },
};
