/*************************************************************************************************************
 * Title: MySQL Connection Options
 * Description: File to provide a global configuration for DBs.
 * Author: Daniel R. Little
 * Company: Little Squared, Inc.
 * Date: 12/11/2016
 * Comment: There may be a more benefical way "business" apprach to this that actually secures the password.
/*************************************************************************************************************/
const gardenShed = {
    host    : '[host]',
    user    : '[username]',        
    password: '[password]',
    database: '[schema]'
}

module.exports = {
    gardenShedConn : gardenShed
}