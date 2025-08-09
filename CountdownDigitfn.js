functionCountdownDigit({ digit }) {
    return (
        <img 
            src={`/svg/${digit},svg`}
            alt={digit}
            style={{width: 100, height: 100 }}
        />
    );
}