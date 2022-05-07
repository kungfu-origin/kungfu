import blessed, { Widgets } from 'blessed';

export default (screen: Widgets.Screen): Widgets.LoadingElement => {
  return blessed.loading({
    parent: screen,
    top: '100%-5',
    left: '100%-40',
    height: 5,
    align: 'left',
    valign: 'top',
    width: 40,
    tags: true,
    hidden: true,
    border: 'line',
  });
};
