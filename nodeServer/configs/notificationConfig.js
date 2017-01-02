/*************************************************************************************************************
 * Title: Notification Configuration
 * Description: File to provide a global configuration for Notification Processing
 * Author: Daniel R. Little
 * Company: Little Squared, Inc.
 * Date: 01/01/2017
 * Comment: This maybe better served stored in the DB... This works for now.
/*************************************************************************************************************/
const smtpConfig = {
    host: 'SMTP_HOST',
    port: 000,
    secure: false,
    auth: {
        user: 'SMTP_USER_ID',
        pass: 'SMTP_PASSWORD'
    }
}
const fromEmail = "FROM_EMAIL";

module.exports = {
    smtp : smtpConfig,
    fromEmail : fromEmail
}
