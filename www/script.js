// Function to set a cookie
function setCookie(name, value, days) {
    let expires = "";
    if (days) {
        let date = new Date();
        date.setTime(date.getTime() + (days * 24 * 60 * 60 * 1000));
        expires = "; expires=" + date.toUTCString();
    }
    document.cookie = name + "=" + value + "; path=/" + expires;
}

// Function to get a cookie
function getCookie(name) {
    let nameEQ = name + "=";
    let ca = document.cookie.split(';');
    for (let i = 0; i < ca.length; i++) {
        let c = ca[i].trim();
        if (c.indexOf(nameEQ) == 0) return c.substring(nameEQ.length);
    }
    return null;
}

// Track visit count
window.onload = function() {
    let visits = getCookie("visits") || 0;
    visits++;
    setCookie("visits", visits, 7);
    document.getElementById("visit-count").innerText = `Visit count: ${visits}`;
};

function sayHello() {
    const button = document.querySelector(".btn-primary");
    button.innerText = "✅ 클릭 완료!";
    button.style.background = "#4CAF50";
    button.style.color = "white";

    setTimeout(() => {
        button.innerText = "클릭해보세요";
        button.style.background = "#ffcc00";
        button.style.color = "black";
    }, 2000);

    alert("Hello! Welcome to webserv!!");
}
