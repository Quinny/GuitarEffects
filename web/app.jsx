class PedalBoard extends React.Component {
  constructor(props) {
    super(props);
  }

  render() {
    return (<h2>Put yer pedals here</h2>);
  }
}

// Fetch the feed from the API and render it.
$(document).ready(() => {
  ReactDOM.render(
      <PedalBoard />,
      document.getElementById('pedalboard')
  );
});

