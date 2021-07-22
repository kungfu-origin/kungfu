const NodeGlobals = ['module', 'require'];

module.exports = {
    root: true,
    parser: '@typescript-eslint/parser',
    parserOptions: {
        sourceType: 'module',
    },
    rules: {
        'no-unused-vars': [
            'error',
            // we are only using this rule to check for unused arguments since TS
            // catches unused variables but not args.
            {varsIgnorePattern: '.*', args: 'after-used', argsIgnorePattern: '^_'},
        ],
        // most of the codebase are expected to be env agnostic
        'no-restricted-globals': ['error', ...NodeGlobals],
        // forbidden to use export default
        'no-restricted-syntax': [
            0,
            {
                selector: 'ExportDefaultDeclaration',
                message: 'Forbidden to use export default.',
            },
        ],
        camelcase: [
            'error',
            {
                properties: 'never',
            },
        ],
        semi: [0, 'never'],
        eqeqeq: [2, 'allow-null'],
        quotes: ['error', 'single'],
        'no-var': 2,
        'prefer-const': 2,
        'func-style': [0, 'declaration'],
        'comma-dangle': ['error', 'always-multiline'],
    },
    overrides: [
        {
            files: ['.*/*', '*.config.js', '.eslintrc.js', '**/__tests__/**'],
            rules: {
                'object-curly-newline': 'off',
                'no-restricted-globals': 'off',
                'no-restricted-syntax': 'off',
            },
        },
        {
            files: ['.eslintrc.js'],
            rules: {
                'array-element-newline': ['error', 'consistent'],
                'object-curly-newline': ['error', {consistent: true}],
            },
        },
    ],
};