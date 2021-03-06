/*
** APIs
*/

function displayLogin() {
    logBtn = document.getElementById( "loginLogoutButton" )
    if( userSelections.loggedIn == true ) {
        logBtn.setAttribute( "onclick", "logoutOnClick()")
        logBtn.innerHTML = "Logout"
    }
    else {
        logBtn.setAttribute( "onclick", "loginOnClick()")
        logBtn.innerHTML = "Login"
    }
    $("#loginModalDialog").modal()
}

function handleSessInfo( sess ) {
    console.log( "Handling sess info" );
    logp = document.getElementById( "loginDisplay" );
    if( sess.hasOwnProperty( "privLevel" ) ) {
        userSelections.privLevel = sess.privLevel;
        if( sess.privLevel > 0 ) {
            userSelections.loggedIn = true;
            if( sess.hasOwnProperty( "username" ) ) {
                if( sess.hasOwnProperty( "greeting" ) ) {
                    logp.innerHTML = sess.greeting + sess.username;
                }
                else {
                    logp.innerHTML = sess.username;
                }
            }
            else {
                if( logp.innerHTML == "Login" ) {
                    logp.innerHTML = "Logged In";
                }
            }
        }
        else {
            userSelections.loggedIn = false;
            logp.innerHTML = "Login";
        }
    }
    else {
        userSelections.loggedIn = false;
        logp.innerHTML = "Login"
    }
}

function loginOnClick() {
    console.log( "Login()" );

    if( (document.getElementById( "loginUserName" ).value != "") ) {
        document.getElementById( "loginUserName" ).setAttribute( "class", "inputBorder" );
        if( (document.getElementById( "loginPasswd" ).value != "")  ) {
            document.getElementById( "loginPasswd" ).setAttribute( "class", "inputBorder" );
            console.log( "Got un/pw" );

            $.ajax( {
                type: "POST",
                url: "/api/login",
                dataType: 'json',
                contentType: 'application/json',
                data: JSON.stringify( { username: document.getElementById( "loginUserName" ).value,
                                        password: document.getElementById( "loginPasswd" ).value
                                      } ),
                success: function( data, status, xhr ) {
                    console.log( "login() Success!" );
                    handleSessInfo( data );
                    if( data.result == "OK" ) {
                        toggleLoginDisplay();
                    }
                },
                error: function( xhr, textStatus, errorMsg ) {
                    console.log( "Error: Login return - " + errorMsg );
	            }
            });
            document.getElementById( "loginUserName" ).value = "";
            document.getElementById( "loginPasswd" ).value = "";
        }
        else {
            document.getElementById( "loginPasswd" ).setAttribute( "class", "errorInputBorder" );
        }
    }
    else {
        document.getElementById( "loginUserName" ).setAttribute( "class", "errorInputBorder" );
    }
}

function logoutOnClick() {
    console.log( "Logging out" );
    userSelections.loggedIn = false;
    document.getElementById( "loginDisplay" ).innerHTML = "Login";

    $.ajax( {
        type: "POST",
        url: "/api/logout",
        dataType: 'json',
        contentType: 'application/json',
        data: JSON.stringify( {} ),
        error: function( xhr, textStatus, errorMsg ) {
            console.log( "Error: Logout return - " + errorMsg );
        }
    });
    toggleLoginDisplay();
    changeMode();
}

function changeMode() {
    /*
    td = document.getElementById( "setpointControlTd" );
    while( td.firstChild ) {
        td.removeChild( td.firstChild );
    }
    knob = createKnob();
    */

    if( (userSelections.cntlMode == "MONITOR") && (userSelections.privLevel >= 3) ) {
        userSelections.cntlMode = "CONTROL";
        $( ".controlEnabled" ).prop( "disabled", false );
        processKnobEvent( 4 ); /* Monitor Event */

    }
    else
    {
        userSelections.cntlMode = "MONITOR";
        $( ".controlEnabled" ).prop( "disabled", true );
        /* knob.setAttribute( "data-readonly", "readonly" ); */
        processKnobEvent( 3 ); /* Control Event */
    }

    /*
    td.appendChild( knob );
    $(".knob").knob();
    */


    document.getElementById( "modeDisplay" ).innerHTML = userSelections.cntlMode
}