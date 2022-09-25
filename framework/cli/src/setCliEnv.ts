import dayjs from 'dayjs';
process.env.APP_ID = dayjs().format('MMDDHHmmss');
process.env.BY_PASS_RESTORE = true;
process.env.BY_PASS_ACCOUNTING = true;
process.env.BY_PASS_TRADINGDATA = true;
