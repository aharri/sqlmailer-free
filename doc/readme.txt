Usage: sqlmailer --config <file>

Configuration file:
 <smtp>
   - host and port are both mandatory.

 <mailer>
   - recipients option per_run is mandatory. Option tells to mailer that
     only specific number of recipients is handled. If one mail contains more
     than option declares, it will send it, but won't send next one in queue.

 <servers> 
   - On the moment only one mysql server is supported, 
     but the option for multiple server has been added here for future

 <server>
   - server compress takes false/true for argument
   - timeout is declared in seconds

 <server-options>
   - Options user and password are both mandatory.

 <server-queues>
   - multiple mail queue are supported in one server. See example how to add
   - db and prefix are both mandatory details.

Configuration file example:
<?xml version="1.0" encoding="UTF8"?>
<sqlmailer version="1.0">
        <smtp host="localhost" port="25"/>
        <mailer>
                <rcpts per_run="50"/>
        </mailer>
        <servers>
                <server host="localhost" port="3306" socket="" compress="true" timeout="60">
                        <options>
                                <option name="user"     value="toor"  />
                                <option name="pass"     value="foo"   />
                        </options>
                        <queues>
                                <queue db="openhosting" prefix="demoshop_" />
                                <queue db="openhosting" prefix="mp_" />
                        </queues>
                </server>
        </servers>
</sqlmailer>

