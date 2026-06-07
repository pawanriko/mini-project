function updateClock() {

    const now = new Date();

    let hours = now.getHours();
    let minutes = String(now.getMinutes()).padStart(2, "0");
    let seconds = String(now.getSeconds()).padStart(2, "0");

    let greeting = "";

    if(hours < 12){
        greeting = "Good Morning";
    }
    else if(hours < 18){
        greeting = "Good Afternoon";
    }
    else{
        greeting = "Good Evening";
    }

    document.getElementById("greeting").textContent =
        greeting + ", Pawan";

    let displayHours = String(hours).padStart(2, "0");

    document.getElementById("time").textContent =
        `${displayHours}:${minutes}:${seconds}`;

    const options = {
        weekday: "long",
        day: "numeric",
        month: "long",
        year: "numeric"
    };

    document.getElementById("date").textContent =
        now.toLocaleDateString("en-IN", options);
}

updateClock();

setInterval(updateClock, 1000);
