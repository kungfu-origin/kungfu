import { DEFAULT_PADDING } from '../config/index';
import blessed, { Widgets } from 'blessed';

class Dashboard {
  screen: Widgets.Screen;
  message: Widgets.MessageElement;

  constructor() {
    this.screen = blessed.screen({
      smartCSR: true,
      fullUnicode: true,
      autoPadding: true,
      resizeTimeout: 500,
    });

    this.screen.title = 'Account Dashboard';
  }

  init() {
    console.error('Init must be cover!');
  }

  refresh() {
    console.error('Refresh must be cover!');
  }

  bindEvent() {
    console.error('BindEvent must be cover!');
  }

  render() {
    this.screen.render();
    // async refresh of the ui
    setInterval(function () {
      this.screen.render();
    }, 300);
  }

  initMessage() {
    this.message = blessed.message({
      parent: this.screen,
      top: '100%-5',
      left: '100%-30',
      height: 5,
      padding: DEFAULT_PADDING,
      align: 'left',
      valign: 'middle',
      width: 30,
      tags: true,
      hidden: true,
      border: 'line',
    });
  }
}

export default Dashboard;
