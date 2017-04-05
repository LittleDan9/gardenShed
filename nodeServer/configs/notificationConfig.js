/*************************************************************************************************************
 * Title: Notification Configuration
 * Description: File to provide a global configuration for Notification Processing
 * Author: Daniel R. Little
 * Company: Little Squared, Inc.
 * Date: 01/01/2017
 * Comment: This maybe better served stored in the DB... This works for now.
/*************************************************************************************************************/
const smtpConfig = {
    host: 'smtp.gmail.com',
    port: 465,
    secure: true,
    auth: {
        user: 'gardenshed@littlerichele.com',
        pass: 'lkcqmqvhuqbltxje'
    }
}
const fromEmail = "gardenshed@littlerichele.com";

module.exports = {
    smtp : smtpConfig,
    fromEmail : fromEmail
}
